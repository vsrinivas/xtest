#include <sys/param.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <strings.h>

#define N (1)
static unsigned long gtabi[N];
static unsigned long gtab[N];

unsigned long gcd(unsigned long x, unsigned long y) {
	unsigned long i;

	for (i = MIN(x, y); i >= 1; i--) {
		if ((x % i) == 0 && (y % i) == 0)
			return i;
	}
	return 1;
}

static int print = 0;
unsigned long gcdfast(unsigned long x, unsigned long y) {
	unsigned long t;

	while (y) {
		t = y;
		y = x % y;
		x = t;
	}
	if (print) printf("  %lu \n", x);	/* almost always 1 */
	return x;
}

__attribute__((pure))
unsigned long g(unsigned long n) {
	if (n == 4)
		return 13;
	return g(n - 1) + gcdfast(n, g(n - 1));
}

unsigned long gfast(unsigned long n) {
	unsigned long v;
	if (n == 4)
		return 13;
	if (gtabi[0] == n)
		return gtab[0];

	v = gfast(n - 1) + gcdfast(n, gfast(n - 1));
	gtabi[0] = n;
	gtab[0] = v;
	return v;
}

int main(int argc, char *argv[]) {
	unsigned int n = 15;
	if (argc > 1)
		n = atoi(argv[1]);

	printf("%d \n", gcd(10, 8));

	/* Build memo 'table' */
	unsigned long target = 10000;
	for (unsigned long i = 4; i <= pow(10, n); i++) {
		if (i == target) {
			time_t now = time(NULL);
			char *p = ctime(&now);
			char *q = index(p, '\n');
			*q = 0;
			printf("%s %lu %lu\n", p, i, gtab[0]);
			target *= 10;
	         }

		gfast(i);
		print = 0;
	}

	//printf("%ld\n", gfast(pow(10, 15)));
}
