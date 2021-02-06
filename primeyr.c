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
static inline int isprime(unsigned long l) {
	return invIdx[l];
}


main(argc, argv)
	int argc;
	char *argv[];
{
	int n;
	unsigned long i;

	unsigned long limit = 10000000;
	primetab.s = calloc(limit, sizeof(unsigned long));
	primetab.i = 0;

	sieveOfErat(&primetab, limit);
	invIdx = calloc(limit, sizeof(char));
	for (i = 0; i < primetab.i; i++) {
		invIdx[primetab.s[i]] = 1;
	}

	for (i = 1900; i < 2020; i++) {
		if (isprime(i))
			printf("%d\n", i);
	}
}

