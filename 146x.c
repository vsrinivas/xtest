  #include <sys/mman.h>
unsigned char *invIdx;
unsigned long invIdxLimit = 0;


void sieve(unsigned char *invIdx, unsigned long limit) {
	unsigned long i;
	unsigned long j;

	invIdx[2] = 1;
	for (i = 3; i < limit; i += 2)		// Start w/ all #'s prime
		invIdx[i] = 1;

	unsigned long llimit = sqrt(limit);	// was i*i < limit
#pragma omp parallel for
	for (i = 2; i < llimit; i++) {
		if (invIdx[i]) {
			for (j = i * i; j < limit; j += i) {
				invIdx[j] = 0;
			}
		}
	}
}

unsigned long modexp(unsigned long base, unsigned long exp, unsigned long mod) {
	if (mod == 1) return 0;

	__int128 result = 1;
	__int128 b = base % mod;
	while (exp > 0) {
		if (exp % 2 == 1)
			result = (result * b) % mod;
		exp >>= 1;
		b = (b * b) % mod;
	}
	return result;
}

int isprime(unsigned long n) {
	if (n < invIdxLimit)
		return invIdx[n];
	if ((n % 2) == 0 || (n % 3) == 0)
		return 0;
	if (n <= 3)
		return n > 1;
	if (n % 5 == 0) return 0;
	if (n % 7 == 0) return 0;
	if (n % 11 == 0) return 0;
	if (n % 13 == 0) return 0;
	if (n % 17 == 0) return 0;
	if (modexp(2, n-1, n) != 1) return 0;

	for (unsigned long i = 2; i * i <= n; i++) {
		if (isprime(i)) {
			if (n % i == 0)
				return 0;
		}
	}
	return 1;
}


int check(unsigned long n) {
	unsigned long n2 = n*n;
	unsigned long i;
	for (i = 1; i <= 27; i+=2) {
		int v = isprime(n2 + i);
		if ((i == 1 || i == 3 || i == 7 || i == 9 || i == 13 || i == 27)) {
			if (!v) {
				return 0;
			}
		} else if (v) {
			return 0;
		}
	}
	return 1;
}

main(argc, argv)
	int argc;
	char *argv[];
{
	unsigned long i;
	unsigned long sum = 0;
	unsigned long M = 1000000ul;
	invIdxLimit = 4096 * M;
	invIdx = calloc(invIdxLimit, sizeof(char));
	printf("sieve\n");
	sieve(invIdx, invIdxLimit);
	madvise(invIdx, invIdxLimit * sizeof(char), MADV_MERGEABLE);

	printf("check\n");
#pragma omp parallel for
	for (i = 9; i < 1 * M; i++) {
		if (check(i)) {
#pragma omp atomic
			sum += i;
		}
	}
	printf("sum %lu\n", sum);
}
