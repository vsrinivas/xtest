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
#include <db.h>

#include <leveldb/db.h>


static DB *db;
static int nFiles;

/* leveldb_to_bdb <leveldb> <bdbfile> */
int main(int argc, char* argv[])
{
	int i;

	leveldb::DB* leveldb;
	leveldb::Options options;
	options.create_if_missing = false;
	leveldb::Status status = leveldb::DB::Open(options, argv[1], &leveldb);
	if (!status.ok()) {
		printf("Error opening leveldb\n");
		return -1;
	}

	db = dbopen(argv[2], O_RDWR | O_CREAT, 0777, DB_BTREE, NULL);
	if (!db) {
		printf("Error creating db\n");
		return -1;
	}

	leveldb::Iterator* it = leveldb->NewIterator(leveldb::ReadOptions());
	for (it->SeekToFirst(); it->Valid(); it->Next()) {
		DBT key, value;
		key.data = (void*) it->key().data();
		key.size = it->key().size();
		value.data = (void*) it->value().data();
		value.size = it->value().size();
		int rc = db->put(db, &key, &value, 0);
		if (rc) {
			printf("error\n");
			return -1;
		}
		++nFiles;
	}

	printf("%d files\n", nFiles);
	delete leveldb;
	db->sync(db, 0);
	db->close(db);
}
