#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <ftw.h>
#include <pthread.h>
#include <atomic>
#include <unordered_set>
#include "pwq.h"
#include "md5.h"

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

static char *src;
static char *dst;
static pthread_mutex_t mtx;
static std::unordered_set<char*, Hash, Equal> md5set;
static std::atomic<uint64_t> nFiles;
static std::atomic<uint64_t> nUnique;
static std::atomic<uint64_t> nDuplicate;
static std::atomic<uint64_t> Seq;

// Pipeline:
// 	1. md5								x
// 	2. check for uniqueness						x
//	3. if unique:
//		. find JPG partner
//		. allocate new filenames for file, jpg partner;
//		. copy to dst.

// Per-file handler:
int processfile(const char *fpath, const struct stat *sb, int typeflag) {
	const char *p;

	// Filter for Canon CR2 real files.
	if (typeflag != FTW_F)
		return 0;

	// Have '.cr2'?
	p = strcasestr(fpath, ".CR2");
	if (!p)
		return 0;
	// End in CR2?
	if (p[4] != 0)
		return 0;

	nFiles++;

	char *dfpath = strdup(fpath);
	PWQ::Run([dfpath]() {
		char* hval = md5(dfpath);
		bool found;

		pthread_mutex_lock(&mtx);
		found = (md5set.find(hval) != md5set.end());
		if (!found) {
			nUnique++;
			md5set.insert(hval);
		}
		pthread_mutex_unlock(&mtx);

		if (!found) {
			// Copy the CR2;
			char cpbuf[511];
			char *spath = rindex(dfpath, '/');
			assert(spath);
			char *filename = strdup(spath + 1);	// just the raw filename
			char *q = rindex(filename, '.');
			assert(q);
			*q = 0;

			bzero(cpbuf, sizeof(cpbuf));
			auto seq = Seq++;
			sprintf(cpbuf, "cp \"%s\" %s/%s.%lu.CR2", dfpath, dst, filename, seq);
			printf("> %s\n", cpbuf);
			system(cpbuf);

			// Copy the JPG.
			bzero(cpbuf, sizeof(cpbuf));
			char *r = rindex(dfpath, '.');
			r[1] = 'J'; r[2] = 'P'; r[3] = 'G';
			if (access(dfpath, R_OK) != 0) {
				r[1] = 'j'; r[2] = 'p'; r[3] = 'g';
			}
			sprintf(cpbuf, "cp \"%s\" %s/%s.%lu.JPG", dfpath, dst, filename, seq);
			free(filename);
			printf("> %s\n", cpbuf);
			system(cpbuf);
		} else {
			nDuplicate++;
			free(hval);
		}
		free(dfpath);
	});

	return 0;
}



// a.out <src> <dst>
// For every Canon CR2 in the src, see if its unique; if so, copy it and any
// associated JPG to dst
int main(int argc, char *argv[]) {
	if (argc < 3)
		return -1;

	src = argv[1];
	dst = argv[2];
	int rc = ftw(src, processfile, 100);

	PWQ::Flush();
	printf("%lu files, %lu unique, %lu duplicate\n", nFiles.load(), nUnique.load(), nDuplicate.load());

	for (auto h : md5set) {
		free(h);
	}
	md5set.clear();
	return 0;
}
