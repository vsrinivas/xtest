#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <limits.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <openssl/md5.h>
#include "md5.h"

static char *md5sumbuflocal(const char *buf, size_t size) {
	char sum[2 * MD5_DIGEST_LENGTH + 1] = {};
	unsigned char hash_buf[MD5_DIGEST_LENGTH] = {};
	int i, j;

	MD5_CTX hash_context;
	MD5_Init(&hash_context);
	MD5_Update(&hash_context, buf, size);
	MD5_Final(hash_buf, &hash_context);

	for (i = 0, j = 0; i < MD5_DIGEST_LENGTH; i++)
		j += sprintf(&sum[j], "%02x", hash_buf[i]);
	return strdup(sum);
}

/* MD5 a file */
extern "C"
char *md5sumf(int fd, const struct stat *sb) {
	char bounce[262144];
	char *p; 
	char *q; 
	bool do_mmap;
	int rc;

	do_mmap = !(sb->st_size < sizeof(bounce));

	if (do_mmap) {
		p = (char*) mmap(0, sb->st_size, PROT_READ, MAP_FILE|MAP_SHARED|MAP_POPULATE, fd, 0);
		if (p == MAP_FAILED) {
			close(fd);
			return NULL;
		}
	} else {
		rc = pread(fd, bounce, sb->st_size, 0);
		if (rc != sb->st_size) {
			close(fd);
			return NULL;
		}
		p = (char *) bounce;
	}

	q = md5sumbuflocal((const char *) p, sb->st_size);
	if (do_mmap) {
		munmap(p, sb->st_size);
	}
	close(fd);
	return q;
}

