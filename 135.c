#include <stdio.h>
#include <stdlib.h>

struct seq {
	unsigned long *s;
	unsigned long i;
	unsigned long len;
};

void addel(struct seq *s, unsigned long l) {
	if (s->i == s->len)
		s->s = realloc(s->s, s->len * 1.2);
        s->s[s->i++] = l;
}

int xfind(struct seq *s, unsigned long v) {
	for (unsigned long i = 0; i < s->i; i++) {
		if (s->s[i] == v)
			return 1;
	}
	return 0;
}

void xinit(struct seq *s, unsigned long len) {
	free(s->s);
	s->len = len;
	s->i = 0;
	s->s = calloc(len, sizeof(unsigned long));
}

int main(int argc, char *argv[])
{
	struct seq s = {};
	int seqlen;
	int base;
	unsigned long v;


	for (seqlen = 1; seqlen < 1000; seqlen++) {
		xinit(&s, 1024);
		for (base = 6; base < 1000; base++) {
			v = (base + seqlen + seqlen) * (base + seqlen + seqlen);
			v -= (base + seqlen) * (base + seqlen);
			v -= (base) * (base);

			if (v == 15) printf("%lu %lu\n", base, seqlen);

			if (xfind(&s, v)) {
				printf("%lu : base= %lu seqlen = %lu\n", v, base, seqlen);
				goto out;
			}
			addel(&s, v);
		}
	}
out:
	return 0;
}
