#include <atomic>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <pthread.h>
#include <cassert>
#include <vector>
#include <unordered_set>
#include <ftw.h>
#include "barrier.h"
#include "md5.h"
#include "pwq.h"

struct Hash {
	size_t operator()(char *s) const {
		char buf[8 + 1] = {};
		bcopy(s, buf, 8);
		return strtol(buf, NULL, 16);
	}
};
struct Equal {
	bool operator()(char* const& s1, char* const& s2) const {
		return strcmp(s1, s2) == 0;
	}
};

static Barrier ref_barrier;
static pthread_mutex_t ref_lock;
static std::unordered_set<char*, Hash, Equal> ref_set;
static int ref_nFiles;
static int ref_nDuplicate;

int process_ref_dir(const char *fpath, const struct stat *sb, int typeflag) {
	if (typeflag != FTW_F)
		return 0;
	if ((sb->st_mode & S_IFMT) != S_IFREG)
		return 0;

	ref_nFiles++;

	char* dfpath = strdup(fpath);
	auto fn = [dfpath]() {
		char* hval = md5sum(dfpath);
		bool found;

		pthread_mutex_lock(&ref_lock);
		found = (ref_set.find(hval) != ref_set.end());
		if (!found) {
			ref_set.insert(hval);
		} else {
			ref_nDuplicate++;
			free(hval);
		}
		pthread_mutex_unlock(&ref_lock);

		free(dfpath);
	};
	if (sb->st_size < 262144)
		fn();
	else
		PWQ::Run(fn, &ref_barrier);

	return 0;
}

static std::atomic<int> nRecords;
static std::atomic<int> nMisses;
static Barrier candidate_barrier;

int process_candidate_dir(const char *fpath, const struct stat *sb, int typeflag) {
	if (typeflag != FTW_F)
		return 0;
	if ((sb->st_mode & S_IFMT) != S_IFREG)
		return 0;

	nRecords++;

	char* dfpath = strdup(fpath);
	auto fn = [dfpath]() {
		char* hval = md5sum(dfpath);
		bool found;

		found = (ref_set.find(hval) != ref_set.end());
		if (!found) {
			printf("%s %s\n", dfpath, hval);
			nMisses++;
		}
		free(hval);
		free(dfpath);
	};
	if (sb->st_size < 262144)
		fn();
	else
		PWQ::Run(fn, &candidate_barrier);
	return 0;
}

/* $Id$ */
//  transactor <ref directory> <n directories>
int main(int argc, char *argv[])
{
	char* ref_dir = argv[1];
	int rc;
	
	ftw(ref_dir, process_ref_dir, 100);
	ref_barrier.Wait();

	printf("ref_nFiles %lu\n", ref_nFiles);
	printf("ref_nDuplicate %lu\n", ref_nDuplicate);
	printf("ref_nUnique set %lu\n", ref_set.size());

	printf("Misses ===>\n");
	for (int i = 2; i < argc; i++) {
		char* candidate_path = argv[i];
		PWQ::Run([candidate_path]() {
			ftw(candidate_path, process_candidate_dir, 100);
		}, &candidate_barrier);
	}
	candidate_barrier.Wait();

	printf("misses: %d\n", nMisses.load());
	printf("nRecords: %d\n", nRecords.load());

	for (auto h : ref_set)
		free(h);
	ref_set.clear();
}

