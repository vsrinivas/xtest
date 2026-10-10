#include <time.h>
#include <math.h>

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

//	if (jacobi(v, 11) == -1)
//		return 0;
	if (jacobi(v, 17) == -1)
		return 0;

	v0 = sqrt(v);
	return (v == (v0 * v0));	
}

main(argc, argv)
	char *argv[];
{
	unsigned long start, end;
	int r;
	unsigned long now(void);

	start = now();
	r = issquare(16384);
	end = now();
	printf("%d %llu cyc\n", r, end-start);

	start = now();
	for (int i = 0; i < 10000; i++)
		r += issquare(i);
	end = now();
	printf("%d %llu cyc\n", r, end-start);
}
