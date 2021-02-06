void sieve(unsigned char *invIdx, unsigned long limit) {
	unsigned long i;
	unsigned long j;
	unsigned long sq_lim;

	invIdx[2] = 1;
	for (i = 3; i < limit; i += 2)		// Start w/ all #'s prime
		invIdx[i] = 1;

	sq_lim = sqrt(limit);
	for (i = 2; i < sq_lim; i++) {
		if (invIdx[i]) {
			for (j = i * i; j < limit; j += i) {
				invIdx[j] = 0;
			}
		}
	}
}

int main(int argc, char *argv[]) {
	unsigned long M = 1000000;
	unsigned char *p = calloc(1 * M, sizeof(char));
	sieve(p, 1 * M);

	for (int i = 0; i < 100; i++)
		printf("%d : %d\n", i, p[i]);
}
