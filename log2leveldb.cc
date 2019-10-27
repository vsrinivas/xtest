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
static int logfd;

/* log2leveldb <log> <dbfile> */
int main(int argc, char *argv[])
{
	int i;

	logfd = open(argv[1], O_RDONLY);
	if (logfd == -1)
		return -1;

	leveldb::Options options;
	options.create_if_missing = true;
	leveldb::Status status = leveldb::DB::Open(options, argv[2], &db);
	if (!status.ok()) {
		printf("Error creating db\n");
		return -1;
	}

	uint64_t seq = 1;
	char seqbuf[16 + 1];
	char linebuf[80];
	FILE *fp = fdopen(logfd, "r");
	char* edit;
	while (!feof(fp)) {
		bzero(seqbuf, sizeof(seqbuf));
		bzero(linebuf, sizeof(linebuf));

		sprintf(seqbuf, "%lx", seq);
		fgets(linebuf, sizeof(linebuf), fp);
		if (strlen(linebuf) == 0)
			break;
		edit = index(linebuf, '\n');
		if (edit)
			*edit = 0;
		printf(">> %s\n", linebuf);
		assert(strlen(linebuf) == 32);

		leveldb::Slice key(seqbuf, strlen(seqbuf) + 1);
		leveldb::Slice value(linebuf, strlen(linebuf) + 1);
		auto s = db->Put(leveldb::WriteOptions(), key, value);
		assert(s.ok());

		seq++;
	}

        delete db;
}
