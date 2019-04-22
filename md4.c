#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <limits.h>

#include <openssl/md4.h>
#include "md4.h"

/* MD4 a file */
char *md4sum(const char *path) {
	FILE* fp;
	unsigned char hash_buf[MD4_DIGEST_LENGTH];
	MD4_CTX hash_context;
	size_t bytes;
	char data[1];
	unsigned char sum[2 * MD4_DIGEST_LENGTH + 1];
	int i, j;
	int error = 0;

	fp = fopen(path, "rb");
	if (!fp)
		return NULL;

	j = 0;
	bytes = 0;
	bzero(data, sizeof(data));
	bzero(sum, sizeof(sum));

	MD4_Init(&hash_context);
	for (;;) {
		bytes = fread(data, sizeof(data), 1, fp);
		if (bytes < sizeof(data)) {
			if (ferror(fp)) {
				// Error!
				error = 1;
				break;
			}
		}

		MD4_Update(&hash_context, data, bytes);
		if (bytes < sizeof(data))
			if (feof(fp))
				break;
	}
	MD4_Final(hash_buf, &hash_context);
	fclose(fp);

	for (i = 0; i < MD4_DIGEST_LENGTH; i++)
		j += sprintf(&sum[j], "%02x", hash_buf[i]);

	if (error)
		return NULL;
	return strdup(sum);
}

char *md4sumbuf(const char *buf, size_t size) {
	unsigned char sum[2 * MD4_DIGEST_LENGTH + 1];
	unsigned char hash_buf[MD4_DIGEST_LENGTH];
	int i, j;

	MD4_CTX hash_context;
	MD4_Init(&hash_context);
	MD4_Update(&hash_context, buf, size);
	MD4_Final(hash_buf, &hash_context);

	for (i = 0, j = 0; i < MD4_DIGEST_LENGTH; i++)
		j += sprintf(&sum[j], "%02x", hash_buf[i]);
	return strdup(sum);
}
