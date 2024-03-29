#define _XOPEN_SOURCE 500
#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unordered_set>
#include <leveldb/db.h>

/* dbtransactor [ref] [src] */
// Checks that every hash in |src| is in |ref|
int main(int argc, char* argv[])
{
	std::unordered_set<std::string> ref_hashes;
	int i;
	int nRefFiles = 0;
	int nDuplicates = 0;
	int nMisses = 0;

	leveldb::DB* ref;
	leveldb::Options ref_options;
	ref_options.create_if_missing = false;
	leveldb::Status status = leveldb::DB::Open(ref_options, std::string(argv[1]), &ref);
	if (!status.ok()) {
		printf("Error opening ref\n");
		return -1;
	}

	leveldb::DB* candidate;
	status = leveldb::DB::Open(ref_options, std::string(argv[2]), &candidate);
	if (!status.ok()) {
		printf("Error opening candidate\n");
		return -1;
	}

	leveldb::ReadOptions r_options;
	r_options.verify_checksums = true;
	r_options.fill_cache = false;
	leveldb::Iterator* it = ref->NewIterator(r_options);
	for (it->SeekToFirst(); it->Valid(); it->Next()) {
		if (!it->status().ok()) {
			printf("Iteration error!\n");
			break;
		}

		bool unique = ref_hashes.insert(it->value().ToString()).second;
		if (!unique)
			++nDuplicates;
		++nRefFiles;
	}
	delete it;
	delete ref;

	it = candidate->NewIterator(r_options);
	for (it->SeekToFirst(); it->Valid(); it->Next()) {
		if (!it->status().ok()) {
			printf("Iteration error!\n");
			break;
		}

		bool found = ref_hashes.count(it->value().ToString()) == 1;
		if (!found) {
			printf("Not found in ref: %s, %s\n",
				it->key().ToString().c_str(),
				it->value().ToString().c_str());
			++nMisses;
		}
	}
	delete it;
	delete candidate;

	printf("nRefFiles %d\n", nRefFiles);							// Files in the reference db
	printf("nDuplicates %d\n", nDuplicates);						// Non-unique files in refdb
	printf("nRefUnique %d\n", ref_hashes.size()); // nDuplicates + this == nRefFiles	// Unique files in refdb
	printf("nMisses %d\n", nMisses);							// Files in candidate not in ref
}
