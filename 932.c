// 2025 #s.

#include <math.h>

unsigned long max(unsigned long x, unsigned long y) { if (x > y) return x; return y; }

int main(int argc, char *argv[]) {
	unsigned long t = 4;
	if (argc > 1) t = atoi(argv[1]);
	unsigned long hlimit = pow(10, t/2) - 1;
	unsigned long a, b;
	unsigned long s = 0;

	for (b = 0; b <= hlimit; b++) {
		if ((sqrt(b) * sqrt(b)) != b)
			continue;
		unsigned long b2 = (b * b) + 2 * b;
		unsigned long ll = max(b2 / hlimit, 1);
		int n = (int) pow(10, (int) trunc(log10(b)) +1);
#pragma omp parallel for reduction(+:s)
		for (a = ll; a <= /*hlimit*/n; a++) {
			unsigned long u = n * a + b;
			//unsigned long v = (a + b) * (a + b);
			if (u - (b*b) == (a*a + 2*a*b)) {
				printf("%lu\n", u);
				s += u;
			}
		}
	}
	printf("  %lu\n", s);
}
