#include <time.h>
#include <math.h>

static unsigned long sqd(int n)
{
	/* sum squares of all divisors */
	unsigned long s = 0;
	int i;
	unsigned long p;
	unsigned long n0 = sqrt(n);

	for (i = 1; i <= n0; i++) {
		if ((n % i) == 0) {
			s += (i * i);
			p = n / i;
			if (p != i)
				s += (p * p);
		}
	}
	return s;
}

static int jacobi(unsigned long v, unsigned long n) {
	int r;

	r = 1;
	v = v % n;

	while (v) {
		while ((v % 2) == 0) {
			v = v / 2;
			int n_mod_8 = n % 8;
			if ((n_mod_8 == 3) || (n_mod_8 == 5))
			       r = -r;
		}

		if (((v % 4) == 3) && ((n % 4) == 3))
			r = -r;

		unsigned long v_old = v;
		v = n % v;
		n = v_old;
	}

	if (n == 1) return r;
	return 0;
}

static int issquare(unsigned long v) {
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

	if (jacobi(v, 11) == -1)
		return 0;
	if (jacobi(v, 17) == -1)
		return 0;

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
#pragma omp parallel for reduction(+: s)
	for (n = 1; n < 64000000; n++) {
		v = sqd(n);
		if (issquare(v)) {
			s += n;
		}

		if ((omp_get_thread_num() == 0) && ((n % 100000) == 0)) {
			now = time(NULL);
			p = ctime(&now);
			q = index(p, '\n');
			*q = 0;
			printf("%s %d\n", p, n);
		}
	}

	printf("%ld\n", s);
}
