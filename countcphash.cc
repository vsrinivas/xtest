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

        fd = open(path, O_RDONLY);
        if (!fd)
                return 0;

        p = (char *) mmap(0, sb->st_size, PROT_READ, MAP_SHARED, fd, 0);
        if (p == MAP_FAILED) {
                close(fd);
                return 0;
        }

        q = FNV32((const char *) p, sb->st_size);
        munmap(p, sb->st_size);
        close(fd);
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
	int i;

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
}
