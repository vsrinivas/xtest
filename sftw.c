#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <pthread.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <db.h>
#include "workqueue.h"

static int cache_hits;

char *cmd = "s3cmd";

struct item {
	char *path;
};

struct db {
	pthread_mutex_t		db_mtx;
	/* URL to Score */
	DB			*hashmap;
};

static struct db sysdb;

static void setup_db() {
	sysdb.hashmap = dbopen("hashmap.dat", O_CREAT|O_RDWR, 0777, DB_BTREE,
			       NULL);
}

static void teardown_db() {
	sysdb.hashmap->sync(sysdb.hashmap, 0);
	sysdb.hashmap->close(sysdb.hashmap);
}

static int match_db(char *url) {
	DBT key, val;
	int i;

	key.data = url;
	key.size = strlen(url) - 1; // makes up for \n
	pthread_mutex_lock(&sysdb.db_mtx);
	do {
		i = sysdb.hashmap->get(sysdb.hashmap, &key, &val, 0);
	} while(0);
	pthread_mutex_unlock(&sysdb.db_mtx);

	if (i == 0)
		return (1);
	return (0);
}

struct rmt_store_db_cb {
	char *url;
	unsigned long long size;
	char *score;
};

static void store_db(char *url, unsigned long long size, char *score) {
	DBT key, value;
	key.data = url;
	key.size = strlen(url);
	value.data = score;
	value.size = strlen(score);
	assert(value.size == 32);
	pthread_mutex_lock(&sysdb.db_mtx);
	do {
		sysdb.hashmap->put(sysdb.hashmap, &key, &value, 0);
		sysdb.hashmap->sync(sysdb.hashmap, 0);
	} while(0);
	pthread_mutex_unlock(&sysdb.db_mtx);
}

static void rmt_store_db1(void *cbp) {
	struct rmt_store_db_cb *cb = cbp;
	store_db(cb->url, cb->size, cb->score);
	free(cb->url);
	free(cb->score);
	free(cb);
}

static void rmt_store_db(char *url, unsigned long long size, char *score, struct barrier *bar) {
	struct rmt_store_db_cb *cb;
	cb = calloc(1, sizeof(*cb));
	cb->url = url;
	cb->size = size;
	cb->score = score;
	workqueue(rmt_store_db1, cb, bar);
}

void process_info(struct item *it, struct barrier *bar) {
	int s;
	char *xcmd;
	FILE *fp;
	int i;
	char linebuf[512];
	char *cp;
	char *p;

	unsigned long long size;
	char *url;
	char *score;
	int match;	

/* MATCH reuses stuff from the map */
#ifdef DEBUG
	printf("Trymatch %s\n", it->path);
#endif
	match = match_db(it->path);
	if (match) {
#ifdef DEBUG
		printf("Already stored %s\n", it->path);
#endif
		free(it->path);
		free(it);
		return;
	}

	url = score = NULL;
	s = strlen(cmd) + strlen(it->path) + 16;
	xcmd = calloc(1, s);
	snprintf(xcmd, s, "%s info %s", cmd, it->path);
/*
	system(xcmd);
 */

	fp = popen(xcmd, "r");
	for (i = 0;; i++) {
		bzero(linebuf, sizeof(linebuf));
		cp = fgets(linebuf, sizeof(linebuf), fp);
		if (cp == NULL)
			break;

		p = index(linebuf, ':');
		switch (i) {
		case 0: /* URL */
			p = strstr(linebuf, "(object)");
			p--;
			url = strndup(linebuf, p-linebuf);
			break;
		case 1: /* size */
			p++;
			size = atoll(p);
			break;
		case 2: /* mtime */
			break;
		case 3: /* mime */
			break;
		case 4: /* md5 */
			p++;
			while (isspace(*p))
				p++;
			score = strndup(p, 32);
			break;
		case 5: /* acl */
			break;
		}
	}
	/* Got a full record; in the future, abort/retry XXX */
//	assert(i == 6);
	if (i != 6) {
		/* incomplete record!! */
		++cache_hits;
		printf("FILE FAILURE =============== %s\n", url);
		goto out;
	}
//	store_db(url, size, score);
	rmt_store_db(url, size, score, bar);

#ifdef DEBUG
	printf("%s %llu %s\n", url, size, score);
#endif

out:
	free(url);
	free(score);

	pclose(fp);

	free(it->path);
	free(it);
	free(xcmd);
}

void process_one_item(struct item *it, struct barrier *bar) {
	int s;
	char *cmdbuf;
	int di = 0, di2 = 0;
	/* Deferred directories */
	struct item **deferred;
	/* Deferred files */
	struct item **deferred2;

	deferred = calloc(1024, sizeof(struct item *));
	deferred2 = calloc(4096, sizeof(struct item *));

	s = strlen(cmd) + strlen(it->path) + 16;
	cmdbuf = calloc(1, s);
	snprintf(cmdbuf, s, "%s ls %s", cmd, it->path);

#ifdef DEBUG
	printf("DO %s\n", cmdbuf);
#endif
	FILE *fp = popen(cmdbuf, "r");
	char linebuf[512];
	char *cp;
	char *dx; // isdir
	char *lpath;

	for (;;) {
		bzero(linebuf, sizeof(linebuf));
		cp = fgets(linebuf, sizeof(linebuf), fp);
		if (cp == NULL)
			break;

		dx = strstr(linebuf, "DIR");
		if (dx) {
			lpath = strstr(linebuf, "s3");
			if (lpath == NULL)
				goto cont;
			/* This is a dir */
			struct item *nit;
			nit = calloc(1, sizeof(*nit));
			nit->path = strdup(lpath);
			deferred[di++] = nit;
	// Defer queueing work. We don't want to get into deep chains and
	// workqueue() can itself process part of the queue.
	//		workqueue(process_one_item, nit, bar);
		} else {
			lpath = strstr(linebuf, "s3");
			if (lpath == NULL)
				goto cont;
			struct item *nit;
			nit = calloc(1, sizeof(*nit));
			nit->path = strdup(lpath);
			deferred2[di2++] = nit;	
		}
cont:
#ifdef DEBUG
		printf(">>%s", cp);
#endif
		;
	}
	pclose(fp);

	int i;
	for (i = 0; i < di; i++) {
		workqueue(process_one_item, deferred[i], bar);
	}
	for (i = 0; i < di2; i++)
		workqueue(process_info, deferred2[i], bar);

	free(deferred);
	free(deferred2);
	free(cmdbuf);
	free(it->path);
	free(it);
}

int main(int argc, char *argv[]) {
	char *base = "s3://d58aab6ea663d41f0cfd6db43aa7d66d/";
	struct barrier bio_wait;
	struct item *it;

	setup_db();

	it = calloc(1, sizeof(*it));
	bzero(&bio_wait, sizeof(bio_wait));
       
	it->path = strdup(base);

	process_one_item(it, &bio_wait);
	bwait(&bio_wait);

	teardown_db();
}
