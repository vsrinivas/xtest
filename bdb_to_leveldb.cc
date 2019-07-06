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
#include <db_185.h>

#include <leveldb/db.h>


static DB *db;
static int nFiles;

/* bdb_to_leveldb <leveldb> <bdbfile> */
int main(int argc, char* argv[])
{
	int i;

	leveldb::DB* leveldb;
	leveldb::Options options;
	options.create_if_missing = true;
	leveldb::Status status = leveldb::DB::Open(options, argv[1], &leveldb);
	if (!status.ok()) {
		printf("Error opening leveldb\n");
		return -1;
	}

	db = ::dbopen(argv[2], O_RDONLY, 0777, DB_BTREE, NULL);
	if (!db) {
		printf("Error creating db\n");
		return -1;
	}

	for (;;) {
		DBT key, value;
		i = db->seq(db, &key, &value, R_NEXT);
		if (i != 0)
			break;

		leveldb::Slice lkey((const char *) key.data, key.size);
		leveldb::Slice lvalue((const char *) value.data, value.size);

		auto s = leveldb->Put(leveldb::WriteOptions(), lkey, lvalue);
		if (!s.ok()) {
			printf("error\n");
			break;
		}

		++nFiles;
	}

	printf("%d files\n", nFiles);
	delete leveldb;
	db->close(db);
}
