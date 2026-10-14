#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

unsigned char *invIdx;
unsigned long invIdxLimit = 0;
typedef __uint128_t uint128;

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

	for (i = 2; i*i < limit; i++) {
		if (bit(invIdx, i)) {
			for (j = i * i; j < limit; j += i) {
				setbit(invIdx, j, 0);
			}
		}
	}
}
void sieve2(unsigned char *invIdx, unsigned long limit) {
        unsigned long i;
        unsigned long j;

        memset(invIdx, 0xff, bits(limit));

        for (j = 3; j < limit; j += 6) setbit(invIdx, j, 0);
 setbit(invIdx, 3, 1);
        for (j = 5; j < limit; j += 10) setbit(invIdx, j, 0);
 setbit(invIdx, 5, 1);

     static const unsigned long gaps[] = {4, 2, 4, 2, 4, 6, 2, 6};
    int gapIdx = 0;

        for (i = 7; i*i < limit; /*i += 2*/) {
                if (bit(invIdx, i)) {
                        for (j = i * i; j < limit; j += (2  *i)) {
                                setbit(invIdx, j, 0);
                        }
                }

                i += gaps[gapIdx];
                gapIdx = (gapIdx + 1) % 8;
        }
}




// The core Miller-Rabin witness test
int miller_rabin_test(unsigned long long n, unsigned long long a) {
    unsigned long long d = n - 1;
    int s = 0;
    while (d % 2 == 0) {
        d /= 2;
        s++;
    }

    unsigned long long x = modexp(a, d, n);
    if (x == 1 || x == n - 1) return 1;

    for (int r = 1; r < s; r++) {
        x = (unsigned long long)((uint128)x * x % n);
        if (x == n - 1) return 1;
    }
    return 0;
}

static int allall, all, misses;
int isprime(unsigned long n) {
	allall++;
	if (n == 0) return 0;
	if (n == 1) return 0;
	if (n == 2) return 1;
	if ((n % 2) == 0) return 0;
	if (n < invIdxLimit)
		return bit(invIdx, n);
	if ((n % 2) == 0 || (n % 3) == 0)
		return 0;
	if (n <= 3)
		return n > 1;
	if (n % 5 == 0) return 0;
	if (n % 7 == 0) return 0;
	if (n % 11 == 0) return 0;
	if (n % 13 == 0) return 0;
	if (n % 17 == 0) return 0;
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

    for (int i = 0; i < 9; i++) {
        if (n == bases[i]) return 1;
        if (!miller_rabin_test(n, bases[i])) return 0;
    }

    return 1;
}

uint128 fact_mod(unsigned long n, unsigned long p) {
	uint128 res = 1;
	for (int i = 1; i <= n; i++) {
		res = (res * i) % p;
	}
	return res;
}

uint128 fact(unsigned long n) {
	if (n == 0)
		return 1;
	return n * fact(n - 1);
}

__attribute__((pure))
unsigned long t(unsigned long p) {
	uint128 sum = 0;
	int k;

//	printf("%lu: ", p);
	for (k = 1; k <= 5; k++) {
		sum += fact_mod(p - k, p);
	}
//	printf("\n");

	return sum % p;
}

int main(int argc, char *argv[])
{
	unsigned long i, j = 0;
	unsigned long sum = 0;
	time_t start, end, now;
	unsigned long v;

	invIdxLimit = 1024ull * 1024 * 1024;
	invIdx = calloc(bits(invIdxLimit), sizeof(char));
	printf("sieve... ");
	start = time(NULL);
	sieve2(invIdx, invIdxLimit);
	end = time(NULL);
	printf(" %lu\n", end-start);

	start = time(NULL);
	unsigned long limit = pow(10, 8);
#pragma omp parallel for reduction(+:sum)
	for (i = 5; i < limit; i += 2) {
		if (omp_get_thread_num() == 0) {
                if ((j % 100000) == 0) {
                        now = time(NULL);
                        char *p = ctime(&now);
                        char *q = index(p, '\n');
                        *q = 0;
                        printf("%s %lu (%lu)\n", p, j, sum);
                }
		j++;
		}
		if (!isprime(i))
			continue;
		sum += t(i);
	}	
	end = time(NULL);
	printf("%lu\n", sum);
	printf(" %lu s\n", end-start);
}
