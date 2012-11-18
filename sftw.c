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
	
	url=score=NULL;
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
	assert(i == 6);
	store_db(url, size, score);
	printf("%s %llu %s\n", url, size, score);
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
	struct item *deferred[1024];
	struct item *deferred2[4096];

	s = strlen(cmd) + strlen(it->path) + 16;
	cmdbuf = calloc(1, s);
	snprintf(cmdbuf, s, "%s ls %s", cmd, it->path);

//	printf("DO %s\n", cmdbuf);
	FILE *fp = popen(cmdbuf, "r");
	char linebuf[512];
	char *cp;
	char *dx; // isdir
	char *lpath;

	bzero(&deferred[0], sizeof(deferred));
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
//		printf(">>%s", cp);
		;
	}
	pclose(fp);

	int i;
	for (i = 0; i < di; i++) {
		workqueue(process_one_item, deferred[i], bar);
	}
	for (i = 0; i < di2; i++)
		workqueue(process_info, deferred2[i], bar);

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
