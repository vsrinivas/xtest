#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define N (65536)

void add1(int *);
void add2(int *);
void add3(int *);
void add4(int *);
void add5(int *);
void add6(int *);
void add7(int *);
void add8(int *);
void add9(int *);

static int rc(void *x, void *y) {
	return rand() & 0x1;
}

void (*fp[N])(int *);
int main(int argc, char *argv[]) {
	void (*fp2[])(int *) = { add1, add2, add3, add4, add5, add6, add7, add8, add9 };
	int i;
	int j;
	int ctr1, ctr2;

	ctr1 = ctr2 = 0;
	for (i = 0; i < N; i++) {
		fp[i] = fp2[i % (sizeof(fp2) / sizeof(fp2[0]))];
	}
	for (i = 0; i < N; i++) {
		fp[i](&ctr1);
	}
	for (j = 0; j < 100; j++) {
		ctr2 = 0;
		qsort(fp, sizeof(fp) / sizeof(fp[0]), sizeof(fp[0]), rc);
		for (i = 0; i < N; i++) {
			fp[i](&ctr2);
		}
		assert(ctr1 == ctr2);
	}
}
