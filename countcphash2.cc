#define _XOPEN_SOURCE 700
#include <sys/types.h>
#include <sys/stat.h>

#include <libgen.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <limits.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ftw.h>
#include <pthread.h>
#include <functional>
#include <unordered_set>
#include <atomic>
#include <deque>

///////////////////////////////////////////////
static std::atomic<bool> g_should_exit;
static std::deque<std::function<void()>> g_work;
static pthread_mutex_t g_mtx;

extern "C" void* defer_main(void* arg) {
	bool latched_should_exit = false;
	for (;;) {
		if (latched_should_exit)
			break;
		latched_should_exit = g_should_exit.load();

		std::deque<std::function<void()>> work;
		pthread_mutex_lock(&g_mtx);
		work.swap(g_work);
		pthread_mutex_unlock(&g_mtx);

		while (!work.empty()) {
			auto thing = work.front();
			thing();
			work.pop_front();
		}
		sleep(3);
	}
}

void defer(std::function<void()> work) {
	pthread_mutex_lock(&g_mtx);
	g_work.push_back(work);
	pthread_mutex_unlock(&g_mtx);
} 

///////////////////////////////////////////////

#define FNV_PRIME_32 16777619
#define FNV_OFFSET_32 2166136261U
uint32_t FNV32(const char *s, size_t len) {             // FNV1a
        uint32_t hash = FNV_OFFSET_32;
        size_t i;
        for(i = 0; i < len; i++) {
                hash = hash ^ (s[i]);
                hash = hash * FNV_PRIME_32;
        }
        return hash;
} 

uint32_t fnvpath(const char *path, const struct stat *sb) {
        char *p;
        uint32_t q;
        int fd;
	size_t sz = sb->st_size; 

        fd = open(path, O_RDONLY);
        if (!fd)
                return 0;

        p = (char *) mmap(0, sz, PROT_READ, MAP_SHARED, fd, 0);
        if (p == MAP_FAILED) {
                close(fd);
                return 0;
        }

        q = FNV32((const char *) p, sz);
	defer([p, sz]() { munmap(p, sz); });
	defer([fd]() { close(fd); });
        return q;
}

/// ----
static int nFiles;
static int nDuplicates;
static int nUnique;
static unsigned long nBytes;
static unsigned long nBytesUnique;
static std::unordered_set<uint32_t> ref_hashes;
static int nSeq;

int cb(const char *path, const struct stat *sb, int typeflag, struct FTW *ft) {
	int rc;

	//printf(">> %s\n", path);

	switch (typeflag) {
	case FTW_F: {
		if ((sb->st_mode & S_IFMT) != S_IFREG)
			break;

                if (sb->st_size == 0)
                        break;

		++nFiles;
		nBytes += sb->st_size;

		uint32_t s = fnvpath(path, sb);
		printf("==> %s, %x\n", path, s);

		if (ref_hashes.find(s) != ref_hashes.end()) {
			nDuplicates++;
		} else {
			ref_hashes.insert(s);
			nUnique++;
			nBytesUnique += sb->st_size;
		}

		break;
	}
	default: {
		break;
	}
	}

	return 0;
}

// Walk argv[1]; sum unique file bytes and total file bytes. The idea is to find how much de-dup would save.

int main(int argc, char *argv[])
{
	pthread_t defer_thr;
	int i;

	pthread_create(&defer_thr, nullptr, defer_main, nullptr);

	i = nftw(argv[1], cb, 64, FTW_PHYS);
	if (i) {
		printf("%d err %d\n", i, errno);
	}

	printf("%d files\n", nFiles);
	printf("%d duplicates\n", nDuplicates);
	printf("%d unique\n", nUnique);
	printf("\n");
	printf("%lu bytes total\n", nBytes);
	printf("%lu bytes unique\n", nBytesUnique);

	g_should_exit.store(true);
	pthread_join(defer_thr, nullptr);
	return 0;
}
