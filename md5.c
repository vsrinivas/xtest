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

	p = mmap(0, sb.st_size, PROT_READ, MAP_FILE|MAP_SHARED|MAP_POPULATE, fd, 0);
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

	MD5_CTX hash_context;
	MD5_Init(&hash_context);
	MD5_Update(&hash_context, buf, size);
	MD5_Final(hash_buf, &hash_context);

	for (i = 0, j = 0; i < MD5_DIGEST_LENGTH; i++)
		j += sprintf(&sum[j], "%02x", hash_buf[i]);
	return strdup(sum);
}
