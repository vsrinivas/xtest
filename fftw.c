#include <sys/types.h>
#include <sys/stat.h>
#include <sys/queue.h>

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <dirent.h>
#include <ftw.h>

#include "fftw.h"
#include "workqueue.h"

struct item {
	char *basepath;
	char *name;
	int (*fn)(const char *, const struct stat *, int, void *);
	void *priv;
};

static void
processdir(const char *path,
	   int (*fn)(const char *, const struct stat *, int, void *),
	   void *priv, struct barrier *bar);

static int
process_one_item(struct item *it, struct barrier *bar) {
	int rv;
	struct stat sb;
	char *namebuf = NULL;
	size_t namelen;
	int flags = 0;

	if (strncmp("..", it->name, strlen("..")) == 0)
		goto out;
	if (strncmp(".", it->name, strlen(".")) == 0)
		goto out;

	namelen = strlen(it->basepath);
	namelen += strlen(it->name);
	namelen += 1;
	namelen += 1;

	namebuf = calloc(1, namelen);
	strcat(namebuf, it->basepath);
	strcat(namebuf, it->name);

	//printf("STAT: %s\n", namebuf);
	rv = stat(namebuf, &sb);
	if (rv == -1) {
		flags = FTW_NS;
		goto issue;
	}

	if (S_ISDIR(sb.st_mode)) {
		flags = FTW_D;
		strcat(namebuf, "/");
		processdir(namebuf, it->fn, it->priv, bar);
	} else {
		flags = FTW_F;
	}

issue:
	it->fn(namebuf, &sb, flags, it->priv);

out:
	free(namebuf);
	free(it->basepath);
	free(it->name);
	free(it);

	return (0);
}

static void
processdir(const char *path,
	   int (*fn)(const char *, const struct stat *, int, void *),
	   void *priv, struct barrier *bar)
{
	DIR *dir;
	struct dirent *de, *buf;
	struct item *item;

	//printf("%s ========>\n", path);

	dir = opendir(path);
	if (dir == NULL)
		return;		

	buf = malloc(offsetof(struct dirent, d_name) +
                     pathconf(path, _PC_NAME_MAX) + 1);

	while (readdir_r(dir, buf, &de) == 0 && de) {
		item = calloc(1, sizeof(struct item));
		item->basepath = strdup(path);
		item->name = strdup(de->d_name);
		item->fn = fn;
		item->priv = priv;
		workqueue(process_one_item, item, bar);
	}

	closedir(dir);
	free(buf);
}

int
fftw(const char *path,
     int (*fn)(const char *, const struct stat *, int, void *),
     void *priv)
{
	struct barrier bar;
	struct item *it;

	bzero(&bar, sizeof(bar));
	it = calloc(1, sizeof(*it));

	it->basepath = strdup(path);
	it->name = strdup("");
	it->fn = fn;
	it->priv = priv;

	process_one_item(it, &bar);

	bwait(&bar);

	return (0);
}

#ifdef TEST

int callx(const char *name, const struct stat *sb, int flag, void *priv) {
	char buf[256];
	printf("%s ===>\n", name);
	if (flag == FTW_F) {
		snprintf(buf, 256, "md5sum \"%s\"", name);
		system(buf);
	}
}

main(int argc, char *argv[]) {
	struct barrier *b = calloc(1, sizeof(*b));
	fftw(argv[1], callx, b);
	bwait(b);
	free(b);
}
#endif
