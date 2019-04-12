#define _XOPEN_SOURCE 500
#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <fcntl.h>
#include <ftw.h>
#include <string.h>

#include <leveldb/db.h>

static leveldb::DB* db;
static int nFiles;

void md5sum(const char *path, char *buf) {
	FILE *mdproc;
	int buflen;
	char *cmdbuf;
	char *readbuf;
	char *p;
	char *q;

	buflen = strlen("md5sum ") + strlen(path) + 1 + 1 + 1 + 1 + 1;
	cmdbuf = (char*) malloc(buflen);
	bzero(cmdbuf, buflen);

	sprintf(cmdbuf, "md5sum \"%s\"", path);

	readbuf = (char*) malloc(10240);
	bzero(readbuf, 10240);
	mdproc = popen(cmdbuf, "r");
	fread(readbuf, 10240, 1, mdproc);
	pclose(mdproc);

	/* 48893e0960c48f79cc455cacd44a6dc0 path */
	p = readbuf;

	strncpy(buf, p, 32);

	free(readbuf);
	free(cmdbuf);
}

int cb(const char *path, const struct stat *sb, int typeflag, struct FTW *ft) {
	char md5[32 + 1];

	//printf(">> %s\n", path);
	bzero(md5, sizeof(md5));

	switch (typeflag) {
	case FTW_F: {
		if ((sb->st_mode & S_IFMT) != S_IFREG)
			break;

		leveldb::Slice key(path, strlen(path) + 1);

		md5sum(path, md5);
		printf("==> %s, %s\n", path, md5);

		leveldb::Slice value(md5, sizeof(md5));

		++nFiles;
		auto rc = db->Put(leveldb::WriteOptions(), key, value);
		break;
	}
	default:
		break;
	}

	return 0;
}

/* ftwdb <path> <dbfile> */
int main(int argc, char *argv[])
{
	int i;

	leveldb::Options options;
	options.create_if_missing = true;
	leveldb::Status status = leveldb::DB::Open(options, argv[2], &db);
	if (!status.ok()) {
		printf("Error creating db\n");
		return -1;
	}

	i = nftw(argv[1], cb, 64, FTW_PHYS);
	if (i) printf("%d err %d\n", i, errno);

        delete db;
	printf("%d files\n", nFiles);
}
