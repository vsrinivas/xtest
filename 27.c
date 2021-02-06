/*
 * changeset:   44:1ca29d1b828a
 * user:        Venkatesh Srinivas <me@endeavour.zapto.org>
 * date:        Mon Jul 04 01:18:28 2011 -0400
 * summary:     10better (faster sieve)
 */

struct seq { unsigned long *s; unsigned long i; };

void addel(struct seq *s, unsigned int l) {
        s->s[s->i++] = l;
}

void sieveOfErat(struct seq *primes, unsigned int lim) {
        unsigned int i = 0;
        unsigned int j = 0;
        unsigned int pi;

        addel(primes, 2);
        addel(primes, 3);
        for (i = 3; i < lim; i+=2)
                if (i % 6 == 1 | i % 6 == 5)
                        addel(primes, i);

        for(i = 1; i < primes->i; i++)
                if (pi = primes->s[i])
                        for (j = i*i; j < primes->i; j++)
                                if (primes->s[j] != 0)
                                        if ((primes->s[j] % pi == 0) &&
                                            (primes->s[j] != pi))
                                                primes->s[j] = 0;
}


/*
 * 11 July 2016
 * 3 Jan 2021 - 
 */

struct seq primetab;
unsigned char *invIdx;
static inline int isprime(signed long l) {
	if (l < 1) return 0;
	return invIdx[l];
}

unsigned long primes_in_a_row(signed long a, signed long b) {
	// number of primes produced in a row by n^2 + an + b;
	signed long n = 0;
	for (n = 0; n < 10000000; n++) {
		signed long poly = n*n + (a*n) + b;
		if  (!isprime(poly))
			break;
	}
	return n;
}

main(argc, argv)
	int argc;
	char *argv[];
{
	unsigned long i;

	unsigned long limit = 10000000;
	primetab.s = calloc(limit, sizeof(unsigned long));
	primetab.i = 0;

	sieveOfErat(&primetab, limit);
	invIdx = calloc(limit, sizeof(char));
	for (i = 0; i < primetab.i; i++) {
		invIdx[primetab.s[i]] = 1;
	}

	unsigned long max_in_a_row = 0, max_a = 0, max_b = 0;
	unsigned long n;

//	test: 80	printf("%lu\n", primes_in_a_row(-79 , 1601)); 

	// Ok, problem #27 - we want to find the number of sequential primes produced by the quadratic.
	for (signed int a = -999; a < 1000; a++) {
		for (signed int b = -1000; b <= 1000; b++) {
			n = primes_in_a_row(a, b);
			if (n > max_in_a_row) {
				max_in_a_row = n;
				max_a = a;
				max_b = b;
			}
		}
	}
	printf("%ld max in a row; %ld a %ld b\n", max_in_a_row, max_a, max_b);
}
