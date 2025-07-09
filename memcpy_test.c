#include <sys/types.h>

#include <stdio.h>
#include <string.h>
#include <assert.h>

void test(void *(*fn)(void *, void *, size_t)) {
	#define UNIT (10240)
	int i, j, cp, off;
	char src[UNIT], dst[UNIT];
	char *bp, *bp2;

	for (i = 0; i < UNIT; i++) {
		for (j = 0; j < UNIT; j++) {
			src[j] = i + j;
		}
		memset(dst, 0, UNIT);

		off = (i & 63);
		bp = src + off;
		bp2 = dst + off;
		cp = (i < (UNIT - off)) ? i : UNIT - off;
		fn(bp2, bp, cp);

		for (j = off; j < cp; j++) {
			assert(bp[j] == bp2[j]);
		}
	}
}


int main(int argc, char *argv[]) {
	printf("memcpy ===>\n");
	test(memcpy);

	printf("_zencpy ===>\n");
	void *_zencpy(void *, void *, size_t);
	test(_zencpy);
	
	printf("_zencpy2 ===>\n");
	void *_zencpy2(void *, void *, size_t);
	test(_zencpy2);
	
	printf("vcopy ===>\n");
	void *vcopy(void *, void *, size_t);
	test(vcopy);
	
	printf("vcopy2 ===>\n");
	void *vcopy2(void *, void *, size_t);
	test(vcopy2);
}
