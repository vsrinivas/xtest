#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <limits.h>

#include <openssl/md5.h>

/* MD5 a file */
char *md5sum(const char *path) {
	FILE* fp;
	unsigned char hash_buf[MD5_DIGEST_LENGTH];
	MD5_CTX hash_context;
	size_t bytes;
	char data[1];
	unsigned char sum[2 * MD5_DIGEST_LENGTH + 1];
	int i, j;
	int error = 0;

	fp = fopen(path, "rb");
	if (!fp)
		return NULL;

	j = 0;
	bytes = 0;
	bzero(data, sizeof(data));
	bzero(sum, sizeof(sum));

	MD5_Init(&hash_context);
	for (;;) {
		bytes = fread(data, sizeof(data), 1, fp);
		if (bytes < sizeof(data)) {
			if (ferror(fp)) {
				// Error!
				error = 1;
				break;
			}
		}

		MD5_Update(&hash_context, data, bytes);
		if (bytes < sizeof(data))
			if (feof(fp))
				break;
	}
	MD5_Final(hash_buf, &hash_context);
	fclose(fp);

	for (i = 0; i < MD5_DIGEST_LENGTH; i++)
		j += sprintf(&sum[j], "%02x", hash_buf[i]);

	if (error)
		return NULL;
	return strdup(sum);
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
