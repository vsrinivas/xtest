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
#include "md5.h"

static leveldb::DB* db;
static int nFiles;

int cb(const char *path, const struct stat *sb, int typeflag, struct FTW *) {
	char* val;
	int rc = 0;

	switch (typeflag) {
	case FTW_F: {
		if ((sb->st_mode & S_IFMT) != S_IFREG)
			break;

		if (sb->st_size == 0)
			break;

		leveldb::Slice key(path, strlen(path) + 1);

		printf("==> %s, ", path);
		if (sb->st_size > 1048576)
			fflush(stdout);
		val = md5sum(path);
		if (!val) {
 			printf("md5sum error: %s\n", path);
			rc = -1;
			break;
		}
		printf("%s\n", val);

		leveldb::Slice value(val, 32 + 1);

		++nFiles;
		auto s = db->Put(leveldb::WriteOptions(), key, value);
		if (!s.ok()) {
 			printf("leveldb error: %s\n", path);
			rc = -1;
		}
		free(val);
		break;
	}
	default:
		break;
	}

	return rc;
}

/* ftwdb2_leveldb <path> <dbfile> */
/* Make a LevelDB mapping every file to its MD5SUM (in 32-byte ascii string) */
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
