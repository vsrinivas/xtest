#define _XOPEN_SOURCE 500
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <fcntl.h>
#include <ftw.h>
#include <string.h>

#include <leveldb/db.h>
#include "hashes.h"

static leveldb::DB* db;
static int nFiles;

char *xhash(const char *path) {
        char *p;
        char *q;
        int fd;
        struct stat sb;
	uint64_t qq;

        fd = open(path, O_RDONLY);
        if (!fd)
                return NULL;

        fstat(fd, &sb);

        p = (char*) mmap(0, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
        if (p == MAP_FAILED) {
                close(fd);
                return NULL;
        }

        qq = FNV1A_64((const char *) p, sb.st_size);
        munmap(p, sb.st_size);
	q = (char*)malloc(17);
	bzero(q, 17);
	sprintf(q, "%16llx", qq);
        close(fd);
        return q;
}


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

		val = xhash(path);
		if (!val) {
 			printf("xhash error: %s\n", path);
			rc = -1;
			break;
		}
		printf("==> %s, %s\n", path, val);

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

// make libleveldb.a
// g++ ftwdb2_fnv.cc -Ithird_party/leveldb/include -std=c++17 libleveldb.a fnv1a.c 

/* ftwdb2_leveldb <path> <dbfile> */
/* Make a LevelDB mapping every file to its FNV1a Hash (in 16-byte ascii string) */
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
