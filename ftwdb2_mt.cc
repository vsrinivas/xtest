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
#include "worker.h"
#include "readn.h"

static Worker *hasher;
static leveldb::DB* db;
static int nFiles;

int cb(const char *path, const struct stat *sb, int typeflag, struct FTW *) {
	ScopedFile* s;

	switch (typeflag) {
	case FTW_F: {
		if ((sb->st_mode & S_IFMT) != S_IFREG)
			break;

		s = new ScopedFile(strdup(path), *sb);
		if (!s->Read()) {
			printf("read error %s\n", path);
			_Exit(-1);
		}

		hasher->Run([s]() {
			leveldb::Slice key(s->path(), strlen(s->path()) + 1);

			char* val = md5sumbuf(s->data(), s->size());
			if (!val) {
 				printf("md5sum error: %s\n", s->path());
				_Exit(-1);
			}
			printf("==> %s, %s\n", s->path(), val);

			leveldb::Slice value(val, 32 + 1);

			++nFiles;
			auto st = db->Put(leveldb::WriteOptions(), key, value);
			if (!st.ok()) {
 				printf("leveldb error: %s\n", s->path());
				_Exit(-1);
			}
			free(val);
			delete s;
		});
		break;
	}
	default:
		break;
	}

	return 0;
}

/* ftwdb2_leveldb <path> <dbfile> */
/* Make a LevelDB mapping every file to its MD5SUM (in 32-byte ascii string) */
int main(int argc, char *argv[])
{
	int i;

	hasher = new Worker();
	hasher->Start();

	leveldb::Options options;
	options.create_if_missing = true;
	leveldb::Status status = leveldb::DB::Open(options, argv[2], &db);
	if (!status.ok()) {
		printf("Error creating db\n");
		return -1;
	}

	i = nftw(argv[1], cb, 64, FTW_PHYS);
	if (i) printf("%d err %d\n", i, errno);

	hasher->Stop();
	delete hasher;
        delete db;

	printf("%d files\n", nFiles);
}
