/*
 * changeset:   44:1ca29d1b828a
 * user:        Venkatesh Srinivas <me@endeavour.zapto.org>
 * date:        Mon Jul 04 01:18:28 2011 -0400
 * summary:     10better (faster sieve)
 */

struct seq { unsigned long *s; unsigned int i; };

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
 */

struct seq primetab;

main(argc, argv)
	int argc;
	char *argv[];
{
	int M = 3;
	int n;
	int factors;
	int matched_factors;
	int i;

	int limit = atoi(argv[1]);
	primetab.s = calloc(limit, sizeof(unsigned long));
	primetab.i = 0;

	sieveOfErat(&primetab, limit);

	printf("%d primes\n", primetab.i);
}

