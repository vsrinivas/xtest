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

int nRecordsSrc;
int nRecordsMerged;

/* dbmerge [dst] [src] */
/* [dst] <- [dst] + [src] */
int main(int argc, char* argv[])
{
	int i;

	leveldb::DB* dst;
	leveldb::Options dst_options;
	leveldb::Status status = leveldb::DB::Open(dst_options, std::string(argv[1]), &dst);
	if (!status.ok()) {
		printf("Error opening dst\n");
		return -1;
	}

	leveldb::DB* src; 
	leveldb::Options src_options;
	src_options.create_if_missing = false;
	status = leveldb::DB::Open(src_options, std::string(argv[2]), &src);
	if (!status.ok()) {
		printf("Error opening src\n");
		return -1;
	}

	leveldb::ReadOptions r_options;
	r_options.verify_checksums = true;
	r_options.fill_cache = false;
	leveldb::Iterator* it = src->NewIterator(r_options);
	for (it->SeekToFirst(); it->Valid(); it->Next()) {
		if (!it->status().ok()) {
			printf("Iteration error!\n");
			break;
		}

		nRecordsSrc++;

		// Already exist? MD5s must match.
		std::string old_src_value;
		auto get_dst = dst->Get(r_options, it->key(), &old_src_value);
		if (get_dst.ok()) {
			bool cmp_ok = (old_src_value == it->value().ToString());
			if (!cmp_ok) {
				printf("md5 mismatch: %s: old hash %s new hash %s\n",
					it->key().ToString().c_str(),
					old_src_value.c_str(),
					it->value().ToString().c_str());
				break;
			}
		} else if (get_dst.IsNotFound()) {
			// Good.
			nRecordsMerged++;
		} else {
			printf("get src err\n");
			break;
		}

		auto put_status = dst->Put(leveldb::WriteOptions(), it->key(), it->value());
		if (!put_status.ok()) {
			break;
		}
	}
	delete it;
	delete src;
	delete dst;

	printf("nRecordsSrc %lu\n", nRecordsSrc);
	printf("nRecordsMerged %lu\n", nRecordsMerged);
}
