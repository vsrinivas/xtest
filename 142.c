#include <time.h>
#include <math.h>
#include <limits.h>

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

__attribute__ ((pure))
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

#if 0
	if (jacobi(v, 11) == -1)
		return 0;
	if (jacobi(v, 17) == -1)
		return 0;
#endif

	v0 = sqrt(v);
	return (v == (v0 * v0));	
}

#define N (1000000)
main(argc, argv)
	char *argv[];
{
	unsigned long x, y, z;
	unsigned long psum = ULONG_MAX;

	for (z = 1; z < N; z++) {
		for (y = (z + 1); y < N; y++) {
			if (!issquare(y + z))
				continue;
			if (!issquare(y - z))
				continue;
			for (x = (y + 1); x < N; x++) {
				if (issquare(x+y) && issquare(x-y) && issquare(x+z) && issquare(x-z)) {
					printf("%lu %lu %lu\n", x, y, z);
					if ((x + y + z) < psum)
						psum = x + y + z;
				}
			}
		}
	}
	printf("%lu\n", psum);
}
