#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

unsigned char *invIdx;
unsigned long invIdxLimit = 0;
unsigned long invIdxLimit1 = 0;

int bit(unsigned char *p, unsigned long i) {
	unsigned long idx = (i / 2) / 8;
	unsigned long bitidx = (i / 2) % 8;
	
	if ((i % 2) == 0) return 0;
	return (p[idx] & (1 << bitidx)) != 0;
}

void setbit(unsigned char *p, unsigned long i, int v) {
	unsigned long idx = (i / 2) / 8;
	unsigned long bitidx = (i / 2) % 8;

	if ((i % 2) == 0) return;

	if (v) {
		p[idx] |= (1 << bitidx);
	} else {
		p[idx] &= ~(1 << bitidx);
	}
}

unsigned long bits(unsigned long i) {
	return (i / 2) / 8;
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


void sieve(unsigned char *invIdx, unsigned long limit) {
	unsigned long i;
	unsigned long j;

	memset(invIdx, 0xff, bits(limit));

	for (i = 3; i*i < limit; i += 2) {
		if (bit(invIdx, i)) {
			for (j = i * i; j < limit; j += 2 * i) {
				setbit(invIdx, j, 0);
			}
		}
	}
}

// The core Miller-Rabin witness test
int miller_rabin_test(unsigned long long n, unsigned long long a, unsigned long d, int s) {
    unsigned long long x = modexp(a, d, n);
    if (x == 1 || x == n - 1) return 1;

    for (int r = 1; r < s; r++) {
        x = (unsigned long long)((__uint128_t)x * x % n);
        if (x == n - 1) return 1;
    }
    return 0;
}

static int isprimefast(unsigned long n) {
        int quickcheck[] = {2, 3, 5, 7, 11, 13, 17, 19, 23};
        int i;

        for (i = 0; i < sizeof(quickcheck)/sizeof(quickcheck[0]); i++) {
                if ((n % quickcheck[i]) == 0)
                        return 0;
        }
        if (n < invIdxLimit)
                if (!bit(invIdx, n))
                        return 0;
        return 1;
}

static int allall, all, misses;
int isprime(unsigned long n) {
	allall++;
	if (n == 0) return 0;
	if (n == 1) return 0;
	if (n == 2) return 1;
	if (isprimefast(n) == 0)
		return 0;
	all++;
//	if (modexp(2, n-1, n) != 1) return 0;

	misses++;
#if 0  // TRIAL DIVISION
	for (unsigned long i = 3; i * i <= n; i+= 2) {
		if ((i < invIdxLimit) && (bit(invIdx, i) == 0))
			continue;
		if (n % i == 0)
			return 0;
	}
#endif
    // These bases are sufficient to be 100% deterministic for all 64-bit ints
    static const unsigned long bases[] = {2, 3, 5, 7, 11, 13, 17, 19, 23};

    unsigned long long d = (n - 1);
    int s = 0;
    while (d % 2 == 0) {
      d /= 2;
      s++;
    }
    for (int i = 0; i < sizeof(bases)/sizeof(bases[0]); i++) {
        if (n == bases[i]) return 1;
        if (!miller_rabin_test(n, bases[i], d, s)) return 0;
    }

    return 1;
}

#define MEMO_N (32)
static unsigned long memo2[MEMO_N];
static unsigned int memo2_i;
static void madd2(unsigned long n) {
        memo2[(memo2_i++) & (MEMO_N - 1)] = n;
}
static int mfind2(unsigned long n) {
        int i;
        for (i = 0; i < MEMO_N; i++)
                if (memo2[i] == n)
                        return 1;
        return 0;
}

__attribute__((pure))
unsigned long sum_digs(unsigned long n) {
	unsigned long s = 0;
	while (n) {
		s += (n % 10);
		n = n / 10;
	}
	return s;
}

unsigned long is_harshad(unsigned long n) {
	int x;
	x = n % sum_digs(n);
	return x == 0;
}

__attribute__((pure))
unsigned long is_right_truncatable(unsigned long n) {
	unsigned long n0 = n;
	if (mfind2(n0)) {
		return 1;
	}
	while (n) {
		if (!is_harshad(n))
			return 0;
		n = n / 10;
	}
	madd2(n0);
	return 1;
}

main(argc, argv)
	int argc;
	char *argv[];
{
	unsigned long i, i_trunc, i_trunc_prime;
	unsigned long sum = 0;
	time_t start, end, now;
	unsigned long limit, llimit;
	unsigned long j = 0;

	invIdxLimit = 4096ull * 1024 * 1024;
	invIdxLimit1 = sqrt(invIdxLimit);
	invIdx = calloc(bits(invIdxLimit), sizeof(char));
	printf("sieve... ");
	start = time(NULL);
	sieve(invIdx, invIdxLimit);
	end = time(NULL);
	printf(" %lu\n", end-start);

#if 0
	llimit = 11;
	limit = pow(10, 10);

	llimit = pow(10, 10); sum = 36498117748ul;
	limit = pow(10, 11);

	llimit = pow(10, 11); sum = 497168223439ul;
	limit = pow(10,12);

	llimit = pow(10, 12); sum = 2897368636255ul;
	limit = pow(10, 13);
#endif
	llimit = 11;
	limit = pow(10, 9);

	start = time(NULL);
//#pragma omp parallel for reduction(+: sum)
	for (i = llimit; i < limit; i += 2) {
		if ((omp_get_thread_num() == 0)) {
		       if ((j % 100000000) == 0) {
			now = time(NULL);
			char *p = ctime(&now);
			char *q = index(p, '\n');
			*q = 0;
			printf("%s %lu\n", p, j);
		       }
		       j++;
		}

		i_trunc = i / 10;
		if (!isprimefast(i))
			continue;
		if (!is_harshad(i_trunc))
			continue;
		/* Right truncatable? */
		if (!is_right_truncatable(i_trunc))
			continue;
		/* Prime? */
		if (!isprime(i))
			continue;
		/* Strong? */
		if (!isprime(i_trunc / sum_digs(i_trunc)))
			continue;

		sum += i;
	}
	end = time(NULL);
	printf(" %lu secs\n", end-start);
	printf("%lu\n", sum);
	printf("%d %d %d\n", allall, all, misses);
}
