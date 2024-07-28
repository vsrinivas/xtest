#define _GNU_SOURCE
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

	leveldb::Iterator* it = leveldb->NewIterator(leveldb::ReadOptions());
	for (it->SeekToFirst(); it->Valid(); it->Next()) {
		char* key = strndup(it->key().data(), it->key().size());
		char* value = strndup(it->value().data(), it->value().size());
		printf("%s %s\n", key, value);
		free(key);
		free(value);
		++nFiles;
	}
	delete it;

	printf("%d files\n", nFiles);
	delete leveldb;
}
