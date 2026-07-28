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
#include <deque>
#include <unordered_set>

#include "hashes.h"

uint32_t fnvpath(const char *path, const struct stat *sb) {
        char *p;
        uint32_t q;
        int fd;

        fd = open(path, O_RDONLY);
        if (!fd)
                return 0;

        p = (char *) mmap(0, sb->st_size, PROT_READ, MAP_PRIVATE, fd, 0);
        if (p == MAP_FAILED) {
                close(fd);
                return 0;
        }

        q = FNV1A_32((const char *) p, sb->st_size);
        munmap(p, sb->st_size);
        close(fd);
        return q;
}

/// ----
static int nFilesA, nFilesB;
static unsigned long nBytesA, nBytesB;
static std::unordered_set<uint32_t> hashesA;
static std::unordered_set<uint32_t> hashesB;

int cbA(const char *path, const struct stat *sb, int typeflag, struct FTW *ft) {
	int rc;
	uint32_t s;

	switch (typeflag) {
	case FTW_F: {
		if ((sb->st_mode & S_IFMT) != S_IFREG)
			break;

                if (sb->st_size == 0)
                        break;

		++nFilesA;
		nBytesA += sb->st_size;

		s = fnvpath(path, sb);
		if (s == 0) {
			printf("==> %s,  err %x\n", path, s);
			break;
		}
		printf("==> %s, %x\n", path, s);

		hashesA.insert(s);

		break;
	}
	default: {
		break;
	}
	}

	if (s == 0) return -1;
	return 0;
}
int cbB(const char *path, const struct stat *sb, int typeflag, struct FTW *ft) {
	int rc;
	uint32_t s;

	switch (typeflag) {
	case FTW_F: {
		if ((sb->st_mode & S_IFMT) != S_IFREG)
			break;

                if (sb->st_size == 0)
                        break;

		++nFilesB;
		nBytesB += sb->st_size;

		s = fnvpath(path, sb);
		if (s == 0) {
			printf("==> %s,  err %x\n", path, s);
			break;
		}
		printf("==> %s, %x\n", path, s);

		hashesB.insert(s);

		break;
	}
	default: {
		break;
	}
	}

	if (s == 0) return -1;
	return 0;
}

// Walk argv[1] and argv[2]. Make sure files, bytes, and hashes are all the same.

int main(int argc, char *argv[])
{
	int i;

	i = nftw(argv[1], cbA, 64, FTW_PHYS);
	if (i) {
		printf("%d err %d\n", i, errno);
	}
	i = nftw(argv[2], cbB, 64, FTW_PHYS);
	if (i) {
		printf("%d err %d\n", i, errno);
	}

	if (nFilesA != nFilesB) {
		fprintf(stderr, "nFiles %d %d\n", nFilesA, nFilesB);
		return -1;
	}
	if (nBytesA != nBytesB) {
		fprintf(stderr, "nBytes %ld %ld\n", nBytesA, nBytesB);
		return -2;
	}
	if (hashesA != hashesB) {
		fprintf(stderr, "hashes\n");
		return -3;
	}
	return 0;
}
