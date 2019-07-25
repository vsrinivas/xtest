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
#include <strings.h>
#include <fcntl.h>
#include <ftw.h>
#include <string.h>

#include <string>
#include <unordered_set>

//// MD5------------------------------------------------------------------------
#define MD5_DIGEST_LENGTH	16

 
/* Any 32-bit or wider unsigned integer data type will do */
typedef unsigned int MD5_u32plus;
 
typedef struct {
	MD5_u32plus lo, hi;
	MD5_u32plus a, b, c, d;
	unsigned char buffer[64];
	MD5_u32plus block[16];
} MD5_CTX;
 
/*
 * This is an OpenSSL-compatible implementation of the RSA Data Security, Inc.
 * MD5 Message-Digest Algorithm (RFC 1321).
 *
 * Homepage:
 * http://openwall.info/wiki/people/solar/software/public-domain-source-code/md5
 *
 * Author:
 * Alexander Peslyak, better known as Solar Designer <solar at openwall.com>
 *
 * This software was written by Alexander Peslyak in 2001.  No copyright is
 * claimed, and the software is hereby placed in the public domain.
 * In case this attempt to disclaim copyright and place the software in the
 * public domain is deemed null and void, then the software is
 * Copyright (c) 2001 Alexander Peslyak and it is hereby released to the
 * general public under the following terms:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted.
 *
 * There's ABSOLUTELY NO WARRANTY, express or implied.
 *
 * (This is a heavily cut-down "BSD license".)
 *
 * This differs from Colin Plumb's older public domain implementation in that
 * no exactly 32-bit integer data type is required (any 32-bit or wider
 * unsigned integer data type will do), there's no compile-time endianness
 * configuration, and the function prototypes match OpenSSL's.  No code from
 * Colin Plumb's implementation has been reused; this comment merely compares
 * the properties of the two independent implementations.
 *
 * The primary goals of this implementation are portability and ease of use.
 * It is meant to be fast, but not as fast as possible.  Some known
 * optimizations are not included to reduce source code size and avoid
 * compile-time configuration.
 */

#ifndef HAVE_OPENSSL

#include <string.h>

#include "md5.h"

/*
 * The basic MD5 functions.
 *
 * F and G are optimized compared to their RFC 1321 definitions for
 * architectures that lack an AND-NOT instruction, just like in Colin Plumb's
 * implementation.
 */
#define F(x, y, z)			((z) ^ ((x) & ((y) ^ (z))))
#define G(x, y, z)			((y) ^ ((z) & ((x) ^ (y))))
#define H(x, y, z)			(((x) ^ (y)) ^ (z))
#define H2(x, y, z)			((x) ^ ((y) ^ (z)))
#define I(x, y, z)			((y) ^ ((x) | ~(z)))

/*
 * The MD5 transformation for all four rounds.
 */
#define STEP(f, a, b, c, d, x, t, s) \
	(a) += f((b), (c), (d)) + (x) + (t); \
	(a) = (((a) << (s)) | (((a) & 0xffffffff) >> (32 - (s)))); \
	(a) += (b);

/*
 * SET reads 4 input bytes in little-endian byte order and stores them in a
 * properly aligned word in host byte order.
 *
 * The check for little-endian architectures that tolerate unaligned memory
 * accesses is just an optimization.  Nothing will break if it fails to detect
 * a suitable architecture.
 *
 * Unfortunately, this optimization may be a C strict aliasing rules violation
 * if the caller's data buffer has effective type that cannot be aliased by
 * MD5_u32plus.  In practice, this problem may occur if these MD5 routines are
 * inlined into a calling function, or with future and dangerously advanced
 * link-time optimizations.  For the time being, keeping these MD5 routines in
 * their own translation unit avoids the problem.
 */
#if defined(__i386__) || defined(__x86_64__) || defined(__vax__)
#define SET(n) \
	(*(MD5_u32plus *)&ptr[(n) * 4])
#define GET(n) \
	SET(n)
#else
#define SET(n) \
	(ctx->block[(n)] = \
	(MD5_u32plus)ptr[(n) * 4] | \
	((MD5_u32plus)ptr[(n) * 4 + 1] << 8) | \
	((MD5_u32plus)ptr[(n) * 4 + 2] << 16) | \
	((MD5_u32plus)ptr[(n) * 4 + 3] << 24))
#define GET(n) \
	(ctx->block[(n)])
#endif


/*
 * This processes one or more 64-byte data blocks, but does NOT update the bit
 * counters.  There are no alignment requirements.
 */
static const void *body(MD5_CTX *ctx, const void *data, unsigned long size)
{
	const unsigned char *ptr;
	MD5_u32plus a, b, c, d;
	MD5_u32plus saved_a, saved_b, saved_c, saved_d;

	ptr = (const unsigned char *)data;

	a = ctx->a;
	b = ctx->b;
	c = ctx->c;
	d = ctx->d;

	do {
		saved_a = a;
		saved_b = b;
		saved_c = c;
		saved_d = d;

/* Round 1 */
		STEP(F, a, b, c, d, SET(0), 0xd76aa478, 7)
		STEP(F, d, a, b, c, SET(1), 0xe8c7b756, 12)
		STEP(F, c, d, a, b, SET(2), 0x242070db, 17)
		STEP(F, b, c, d, a, SET(3), 0xc1bdceee, 22)
		STEP(F, a, b, c, d, SET(4), 0xf57c0faf, 7)
		STEP(F, d, a, b, c, SET(5), 0x4787c62a, 12)
		STEP(F, c, d, a, b, SET(6), 0xa8304613, 17)
		STEP(F, b, c, d, a, SET(7), 0xfd469501, 22)
		STEP(F, a, b, c, d, SET(8), 0x698098d8, 7)
		STEP(F, d, a, b, c, SET(9), 0x8b44f7af, 12)
		STEP(F, c, d, a, b, SET(10), 0xffff5bb1, 17)
		STEP(F, b, c, d, a, SET(11), 0x895cd7be, 22)
		STEP(F, a, b, c, d, SET(12), 0x6b901122, 7)
		STEP(F, d, a, b, c, SET(13), 0xfd987193, 12)
		STEP(F, c, d, a, b, SET(14), 0xa679438e, 17)
		STEP(F, b, c, d, a, SET(15), 0x49b40821, 22)

/* Round 2 */
		STEP(G, a, b, c, d, GET(1), 0xf61e2562, 5)
		STEP(G, d, a, b, c, GET(6), 0xc040b340, 9)
		STEP(G, c, d, a, b, GET(11), 0x265e5a51, 14)
		STEP(G, b, c, d, a, GET(0), 0xe9b6c7aa, 20)
		STEP(G, a, b, c, d, GET(5), 0xd62f105d, 5)
		STEP(G, d, a, b, c, GET(10), 0x02441453, 9)
		STEP(G, c, d, a, b, GET(15), 0xd8a1e681, 14)
		STEP(G, b, c, d, a, GET(4), 0xe7d3fbc8, 20)
		STEP(G, a, b, c, d, GET(9), 0x21e1cde6, 5)
		STEP(G, d, a, b, c, GET(14), 0xc33707d6, 9)
		STEP(G, c, d, a, b, GET(3), 0xf4d50d87, 14)
		STEP(G, b, c, d, a, GET(8), 0x455a14ed, 20)
		STEP(G, a, b, c, d, GET(13), 0xa9e3e905, 5)
		STEP(G, d, a, b, c, GET(2), 0xfcefa3f8, 9)
		STEP(G, c, d, a, b, GET(7), 0x676f02d9, 14)
		STEP(G, b, c, d, a, GET(12), 0x8d2a4c8a, 20)

/* Round 3 */
		STEP(H, a, b, c, d, GET(5), 0xfffa3942, 4)
		STEP(H2, d, a, b, c, GET(8), 0x8771f681, 11)
		STEP(H, c, d, a, b, GET(11), 0x6d9d6122, 16)
		STEP(H2, b, c, d, a, GET(14), 0xfde5380c, 23)
		STEP(H, a, b, c, d, GET(1), 0xa4beea44, 4)
		STEP(H2, d, a, b, c, GET(4), 0x4bdecfa9, 11)
		STEP(H, c, d, a, b, GET(7), 0xf6bb4b60, 16)
		STEP(H2, b, c, d, a, GET(10), 0xbebfbc70, 23)
		STEP(H, a, b, c, d, GET(13), 0x289b7ec6, 4)
		STEP(H2, d, a, b, c, GET(0), 0xeaa127fa, 11)
		STEP(H, c, d, a, b, GET(3), 0xd4ef3085, 16)
		STEP(H2, b, c, d, a, GET(6), 0x04881d05, 23)
		STEP(H, a, b, c, d, GET(9), 0xd9d4d039, 4)
		STEP(H2, d, a, b, c, GET(12), 0xe6db99e5, 11)
		STEP(H, c, d, a, b, GET(15), 0x1fa27cf8, 16)
		STEP(H2, b, c, d, a, GET(2), 0xc4ac5665, 23)

/* Round 4 */
		STEP(I, a, b, c, d, GET(0), 0xf4292244, 6)
		STEP(I, d, a, b, c, GET(7), 0x432aff97, 10)
		STEP(I, c, d, a, b, GET(14), 0xab9423a7, 15)
		STEP(I, b, c, d, a, GET(5), 0xfc93a039, 21)
		STEP(I, a, b, c, d, GET(12), 0x655b59c3, 6)
		STEP(I, d, a, b, c, GET(3), 0x8f0ccc92, 10)
		STEP(I, c, d, a, b, GET(10), 0xffeff47d, 15)
		STEP(I, b, c, d, a, GET(1), 0x85845dd1, 21)
		STEP(I, a, b, c, d, GET(8), 0x6fa87e4f, 6)
		STEP(I, d, a, b, c, GET(15), 0xfe2ce6e0, 10)
		STEP(I, c, d, a, b, GET(6), 0xa3014314, 15)
		STEP(I, b, c, d, a, GET(13), 0x4e0811a1, 21)
		STEP(I, a, b, c, d, GET(4), 0xf7537e82, 6)
		STEP(I, d, a, b, c, GET(11), 0xbd3af235, 10)
		STEP(I, c, d, a, b, GET(2), 0x2ad7d2bb, 15)
		STEP(I, b, c, d, a, GET(9), 0xeb86d391, 21)

		a += saved_a;
		b += saved_b;
		c += saved_c;
		d += saved_d;

		ptr += 64;
	} while (size -= 64);

	ctx->a = a;
	ctx->b = b;
	ctx->c = c;
	ctx->d = d;

	return ptr;
}

void MD5_Init(MD5_CTX *ctx)
{
	ctx->a = 0x67452301;
	ctx->b = 0xefcdab89;
	ctx->c = 0x98badcfe;
	ctx->d = 0x10325476;

	ctx->lo = 0;
	ctx->hi = 0;
}

void MD5_Update(MD5_CTX *ctx, const void *data, unsigned long size)
{
	MD5_u32plus saved_lo;
	unsigned long used, available;

	saved_lo = ctx->lo;
	if ((ctx->lo = (saved_lo + size) & 0x1fffffff) < saved_lo)
		ctx->hi++;
	ctx->hi += size >> 29;

	used = saved_lo & 0x3f;

	if (used) {
		available = 64 - used;

		if (size < available) {
			memcpy(&ctx->buffer[used], data, size);
			return;
		}

		memcpy(&ctx->buffer[used], data, available);
		data = (const unsigned char *)data + available;
		size -= available;
		body(ctx, ctx->buffer, 64);
	}

	if (size >= 64) {
		data = body(ctx, data, size & ~(unsigned long)0x3f);
		size &= 0x3f;
	}

	memcpy(ctx->buffer, data, size);
}

#define OUT(dst, src) \
	(dst)[0] = (unsigned char)(src); \
	(dst)[1] = (unsigned char)((src) >> 8); \
	(dst)[2] = (unsigned char)((src) >> 16); \
	(dst)[3] = (unsigned char)((src) >> 24);

void MD5_Final(unsigned char *result, MD5_CTX *ctx)
{
	unsigned long used, available;

	used = ctx->lo & 0x3f;

	ctx->buffer[used++] = 0x80;

	available = 64 - used;

	if (available < 8) {
		memset(&ctx->buffer[used], 0, available);
		body(ctx, ctx->buffer, 64);
		used = 0;
		available = 64;
	}

	memset(&ctx->buffer[used], 0, available - 8);

	ctx->lo <<= 3;
	OUT(&ctx->buffer[56], ctx->lo)
	OUT(&ctx->buffer[60], ctx->hi)

	body(ctx, ctx->buffer, 64);

	OUT(&result[0], ctx->a)
	OUT(&result[4], ctx->b)
	OUT(&result[8], ctx->c)
	OUT(&result[12], ctx->d)

	memset(ctx, 0, sizeof(*ctx));
}

#endif

// XXX
// MD5 ---------------------------------------------------------

char *md5sumat(int fd) {
	char *p; 
	char *q; 
	struct stat sb;
        fstat(fd, &sb);

        p = (char *) mmap(0, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
        if (p == MAP_FAILED) {
                close(fd);
                return NULL;
        }

        q = md5sumbuf((const char *) p, sb.st_size);
        munmap(p, sb.st_size);
        return q;
}

/* MD5 a file */
char *md5sum(const char *path) {
	char *p; 
	char *q; 
	int fd;
	struct stat sb;

	fd = open(path, O_RDONLY);
	if (!fd)
		return NULL;

	fstat(fd, &sb);

	p = (char *) mmap(0, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (p == MAP_FAILED) {
		close(fd);
		return NULL;
	}

	q = md5sumbuf((const char *) p, sb.st_size);
	munmap(p, sb.st_size);
	close(fd);
	return q;
}

char *md5sumbuf(const char *buf, size_t size) {
	unsigned char sum[2 * MD5_DIGEST_LENGTH + 1];
	unsigned char hash_buf[MD5_DIGEST_LENGTH];
	int i, j;

	memset(sum, 0, sizeof(sum));

	MD5_CTX hash_context;
	MD5_Init(&hash_context);
	MD5_Update(&hash_context, buf, size);
	MD5_Final(hash_buf, &hash_context);

	for (i = 0, j = 0; i < MD5_DIGEST_LENGTH; i++) {
		j += snprintf((char *) &sum[j], sizeof(sum) - j, "%02x", hash_buf[i]);
	}
	return strdup((const char *) sum);
}
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

uint32_t fnvpath(const char *path) {
        char *p;
	uint32_t q;
        int fd;
        struct stat sb;

        fd = open(path, O_RDONLY);
        if (!fd)
                return 0;

        fstat(fd, &sb);

        p = (char *) mmap(0, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
        if (p == MAP_FAILED) {
                close(fd);
                return 0;
        }

        q = FNV32((const char *) p, sb.st_size);
        munmap(p, sb.st_size);
        close(fd);
        return q;
}


/// ----
static uint64_t nFiles;
static int nDuplicates;
static int nUnique;
static int nDisaster;
static std::unordered_set<uint32_t> ref_hashes_fnv;
static std::unordered_set<std::string> ref_hashes;
static int dst_fd;
static int nSeq;
static uint64_t nBytesSrc;
static uint64_t nBytesCopied;
static uint64_t nFilesCopied;

int mkpath(int dst, char *dir, mode_t mode)
{
   char* odir = strdup(dir);

   int v = nSeq++;
    //printf("%d: Enter %s\n", nSeq, dir);
    struct stat sb = {};
    if (!dir) {
     //   printf("%d Exit0 %s\n", nSeq, dir);
        errno = EINVAL;
        return 1;
    }
    if (fstatat(dst, dir, &sb, 0) == 0) {
     // printf("%d Exit1 %s\n", nSeq, dir);
      free(odir);
      return 0;
    }
    if (dir[0] == '.' && dir[1] == 0x0) {
      // printf("%d Exit2 %s\n", nSeq, dir);
      free(odir);
      return 0;
    }

    char* dirs = strdup(dir);
    mkpath(dst, dirname(dirs), mode);
    free(dirs);

    //printf("%d Exit %s\n", nSeq, dir);
    int rc = mkdirat(dst, odir, mode);
    free(odir);
    return rc;
}

static int copy(int dst, const char *path, const struct stat *sb, char *src_hash) {
	int sf = open(path, O_RDONLY);
	if (sf == -1)
		return -1;

        char* p = (char *) mmap(0, sb->st_size, PROT_READ, MAP_SHARED, sf, 0);
        if (p == MAP_FAILED) {
                close(sf);
                return -1;
        }

	char* dstf = strdup(path);
	mkpath(dst_fd, dirname(dstf), 0777);
	free(dstf);

	int df = openat(dst, path, O_CREAT | O_EXCL | O_RDWR, sb->st_mode);
	if (df == -1) {
		munmap(p, sb->st_size);
		close(sf);
		return -1;
	}

	int rc = pwrite(df, p, sb->st_size, 0);
	if ((rc == -1) || (rc != sb->st_size)) {
		munmap(p, sb->st_size);
		close(sf);
		close(df);
		return -1;
	}

	munmap(p, sb->st_size);
	close(sf);
	fsync(df);

	char* dsthash = md5sumat(df);
	if (strcmp(src_hash, dsthash) != 0) {
		free(dsthash);
		close(df);
		return -1;
	}

	free(dsthash);
	close(df);
	return 0;
} 

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
		nBytesSrc += sb->st_size;

		char *s = md5sum(path);
		printf("==> %s, %s\n", path, s);

		std::string hash(s);
		if (ref_hashes.find(hash) != ref_hashes.end()) {
			uint32_t fnv = fnvpath(path);
			if (ref_hashes_fnv.find(fnv) == ref_hashes_fnv.end()) {
				// md5 collision?
				++nDisaster;
				return -1;
			}

			nDuplicates++;
		} else {
			ref_hashes.insert(hash);
			uint32_t fnv = fnvpath(path);
			ref_hashes_fnv.insert(fnv);
			nUnique++;
			
			nFilesCopied++;
			nBytesCopied += sb->st_size;
		
			rc = copy(dst_fd, path, sb, /*src_hash=*/s);
			if (rc == -1) {
				return -1;
			}
		}

		free(s);
		break;
	}
	default: {
		break;
	}
	}

	return 0;
}

int prefill(const char *path, const struct stat *sb, int typeflag, struct FTW *ft) {
	int rc;

	//printf(">> %s\n", path);

	switch (typeflag) {
	case FTW_F: {
		if ((sb->st_mode & S_IFMT) != S_IFREG)
			break;

                if (sb->st_size == 0)
                        break;

		char *s = md5sum(path);
		printf("prefill ==> %s, %s\n", path, s);

		std::string hash(s);
		if (ref_hashes.find(hash) != ref_hashes.end()) {
		} else {
			ref_hashes.insert(hash);
		}

		uint32_t fnv = fnvpath(path);
		ref_hashes_fnv.insert(fnv);

		free(s);
		break;
	}
	default: {
		break;
	}
	}

	return 0;
}

// cphash <src directory> <dst directory>
// dest directory must exist.
// Copy from src->dst, doesn't copy file if it exists in dst hash table already.
// skips 0 size files.
int main(int argc, char *argv[])
{
	int i;

	auto dir = opendir(argv[2]);
	dst_fd = dirfd(dir);
	if (dst_fd == -1)
		return -1;

	i = nftw(argv[2], prefill, 64, FTW_PHYS);
	if (i) {
		printf("dst %d err %d\n", i, errno);
	}

	i = nftw(argv[1], cb, 64, FTW_PHYS);
	if (i) {
		printf("%d err %d\n", i, errno);
	}

	closedir(dir);

	printf("%llu files\n", nFiles);
	printf("%d duplicates\n", nDuplicates);
	printf("%d unique\n", nUnique);
	printf("%d MD5 collisions\n", nDisaster);
	printf("Source %llu files (%llu bytes), Copied %llu files (%llu bytes)\n",
		nFiles, nBytesSrc, nFilesCopied, nBytesCopied);
}
