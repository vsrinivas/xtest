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

/* MD5 a file */
char* md5sum(char* path) {
        FILE* fp;
        char buf[511];
        char hash[32 + 1];

        bzero(buf, sizeof(buf));
        sprintf(buf, "md5sum \"%s\"", path);

        fp = popen(buf, "r");
        if (!fp)
                return NULL;

        bzero(md5sum, sizeof(md5sum));
	/* 48893e0960c48f79cc455cacd44a6dc0 path */
        fgets(md5sum, 32, fp);
        pclose(fp);

        if (md5sum[0] == 0)
                return NULL;

        return strdup(md5sum);
}

int cb(const char *path, const struct stat *sb, int typeflag, struct FTW *ft) {
	switch (typeflag) {
	case FTW_F: {
		if ((sb->st_mode & S_IFMT) != S_IFREG)
			break;

		leveldb::Slice key(path, strlen(path) + 1);

		char* md = md5sum(path);
		printf("==> %s, %s\n", path, md5);

		leveldb::Slice value(md, 32);
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
