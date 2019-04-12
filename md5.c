#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <limits.h>

/* MD5 a file */
char* md5sum(char* path) {
	FILE* fp;
	char buf[FILENAME_MAX];
	char md5sum[32 + 1];
	
	bzero(buf, sizeof(buf));
	sprintf(buf, "md5sum \"%s\"", path);

	fp = popen(buf, "r");
	if (!fp)
		return NULL;

	bzero(md5sum, sizeof(md5sum));
	fgets(md5sum, 32, fp);
	pclose(fp);

	if (md5sum[0] == 0)
		return NULL;

	return strdup(md5sum);
}
