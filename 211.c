#include <time.h>
#include <math.h>

unsigned long sqd(int n)
{
	/* sum squares of all divisors */
	unsigned long s = 0;
	int i;

	for (i = 1; i <= n; i++) {
		if ((n % i) == 0)
			s += (i * i);
	}
	return s;
}

int issquare(unsigned long v) {
	int odd = v & 0x1;
	unsigned long v0;

	/* https://en.wikipedia.org/wiki/Square_number */
	if (!odd) {
		/* All squares of even numbers are divisible by 4 */
		if ((v % 4) != 0)
			return 0;
	} else {
		/* All squares of odd numbers % 8 = 1 */
		if ((v % 8) != 1)
			return 0;
	}

	v0 = sqrt(v);
	return (v == (v0 * v0));	
}

main(argc, argv)
	char *argv[];
{
	int n;
	unsigned long s = 0;
	unsigned long v;
	time_t now;
	char *p, *q;

	printf("s2(10) = %ld\n", sqd(10));

	/* Find the sum of all 0 < n < 64 000 000 s.t. sqd(n) is a perfect square */
#pragma omp parallel for
	for (n = 1; n < 64000000; n++) {
		v = sqd(n);
		if (issquare(v)) {
#pragma omp atomic
			s += n;
		}

		if ((n % 10000) == 0) {
			now = time(NULL);
			p = ctime(&now);
			q = index(p, '\n');
			*q = 0;
			printf("%s %d\n", p, n);
		}
	}
}
