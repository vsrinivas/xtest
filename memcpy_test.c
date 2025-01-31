#include <sys/types.h>

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
	test(memcpy);
}
