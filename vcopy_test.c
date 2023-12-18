#include <assert.h>
#include <string.h>

extern void vcopy(void*, void*, unsigned long);
main() {
	char buf[1024];
	char buf2[1024];
	int i;
	int rc;
	int j;

	for (i = 0; i < 1024; i++)
		buf[i] = i + 0xAA;

	for (i = 0; i < 1024; i++) {
		memset(buf2, 0, 1024);
		vcopy(buf2, buf, i);
		rc = memcmp(buf2, buf, i);
		assert(rc == 0);
		for (j = i; j < 1024; j++)
			assert(buf2[j] == 0);
	}

	for (i = 0; i < 1024 - 17; i++) {
		memset(buf2, 0, 1024);
		vcopy(buf2+i, buf+i, 17);
		rc = memcmp(buf2+i, buf+i, 17);
		assert(rc == 0);
		for (j = 0; j < i; j++)
			assert(buf2[j] == 0);
		for (j = i+17; j < 1024; j++)
			assert(buf2[j] == 0);
	}
}
