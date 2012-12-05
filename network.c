
#include <stdio.h>
#include <stdlib.h>

#include "workqueue.h"

struct node {
	char *host;
};

static const char *rcmd = "ssh";

static void do_rcmd(struct node *nd, char *cmd) {
	char *cmdbuf;
	int linelen;
	FILE *fp;
	char *linebuf;
	char *p;

	linelen = snprintf(cmdbuf, 0, "%s %s %s", rcmd, nd->host, cmd) + 1;
	cmdbuf = calloc(1, linelen);
	snprintf(cmdbuf, linelen, "%s %s %s", rcmd, nd->host, cmd);
	fp = popen(cmdbuf, "r");
	free(cmdbuf);

	while (!feof(fp)) {
		linebuf = calloc(1, 80);
		p = fgets(linebuf, 80, fp);
		if (!p)
			break;
		printf(">> %s", linebuf);
		free(linebuf);
	}
	pclose(fp);
}

struct cmd { struct node *n; char *cmd; };

void cmdcb(struct cmd *c, struct barrier *b) {
	do_rcmd(c->n, c->cmd);
}

#ifdef TEST
main() {
	struct barrier b = {};
	struct node n = { "localhost" };
	struct cmd c = { &n, "ls" };
	workqueue(cmdcb, &c, &b);
	struct cmd c2 = { &n, "ls" };
	workqueue(cmdcb, &c2, &b);
	bwait(&b);
}
#endif
