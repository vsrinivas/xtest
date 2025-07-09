#define _XOPEN_SOURCE 700
#include <sys/types.h>
#include <sys/stat.h>

#include <algorithm>
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
#include <strings.h>
#include <fcntl.h>
#include <ftw.h>
#include <string.h>
#include <stdint.h>

/// ------------------------------------- FNV hash
#define FNV_PRIME_32 16777619
#define FNV_OFFSET_32 2166136261U
uint32_t FNV32(const char *s, size_t len) {		// FNV1a
	uint32_t hash = FNV_OFFSET_32;
	size_t i;
	for(i = 0; i < len; i++) {
		hash = hash ^ (s[i]);
		hash = hash * FNV_PRIME_32;
	}
	return hash;
} 

uint32_t fnvpath(int fd) {
        char *p;
	uint32_t q;
        struct stat sb;

        fstat(fd, &sb);

        p = (char *) mmap(0, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
        if (p == MAP_FAILED) {
                close(fd);
                return 0;
        }

        q = FNV32(p, sb.st_size);
        munmap(p, sb.st_size);
        return q;
}

size_t read_bytes = 0;
size_t copied_bytes = 0;

static int copy(const char *path, const char *dst) {
	int sf = open(path, O_RDONLY);
	if (sf == -1)
		return -1;
	struct stat sb;
	fstat(sf, &sb);

        char* p = (char *) mmap(0, sb.st_size, PROT_READ, MAP_SHARED, sf, 0);
        if (p == MAP_FAILED) {
                close(sf);
                return -1;
        }
	madvise(p, sb.st_size, MADV_SEQUENTIAL);

	int df = open(dst, O_RDWR);
	if (df == -1) {
		printf("openat: %d\n", errno);
		munmap(p, sb.st_size);
		close(sf);
		return -1;
	}
	struct stat dstat;
	fstat(df, &dstat);
	if (sb.st_size != dstat.st_size) {
		printf("size? %d\n", errno);
		return -1;
	}

	#define UNIT (1048576ul)
	char tmpbuf[UNIT];
	size_t start;
	for (start = 0; start < sb.st_size; start += UNIT) {
		size_t by = std::min(UNIT, sb.st_size - start);
		bzero(tmpbuf, UNIT);
		ssize_t rc1 = pread(df, tmpbuf, by, start);
		if (rc1 != by) { printf("READ? off %lu\n", start); return -1; }
		int diff = memcmp(tmpbuf, &p[start], by);
		read_bytes += (2 * by);
		if (diff == 0) continue;
		readahead(df, start + by, UNIT);
		ssize_t rc = pwrite(df, &p[start], by, start);
		if (rc != by) {
			printf("failed at off=%lu sz %lu rc %lu\n", start, by, rc);
			return -1;
		}
		copied_bytes += by;
	}

	munmap(p, sb.st_size);
	close(sf);
	fsync(df);

	close(df);
	return 0;
} 

int main(int argc, char *argv[])
{
	int i;

	if (argc != 3)
		return -1;
	i = copy(argv[1], argv[2]);
	printf("read bytes %lu\n", read_bytes);
	printf("copied bytes %lu\n", copied_bytes);
	return i;
}
