#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <pthread.h>
#include <stdlib.h>
#include "workqueue.h"

char *cmd = "s3cmd";

struct item {
	char *path;
};

void process_info(struct item *it, struct barrier *bar) {
	int s;
	char *xcmd;
	s = strlen(cmd) + strlen(it->path) + 16;
	xcmd = calloc(1, s);
	snprintf(xcmd, s, "%s info %s", cmd, it->path);
	system(xcmd);
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

	it = calloc(1, sizeof(*it));
	bzero(&bio_wait, sizeof(bio_wait));
       
	it->path = strdup(base);

	process_one_item(it, &bio_wait);
	bwait(&bio_wait);
}
