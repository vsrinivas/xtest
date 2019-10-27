#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

unsigned __int128 fnv1a_128(unsigned char *s, size_t len) {
	unsigned __int128 kPrime = 16777216ull;
	kPrime <<= 64ull;
	kPrime += (1 << 8);
	kPrime += (0x3b);

	unsigned __int128 kOffset = 0x6c62272e07bb0142ull;
	kOffset <<= 64ull;
	kOffset |= 0x62b821756295c58dull;

	unsigned __int128 hash = kOffset;

        size_t i;
        for(i = 0; i < len; i++) {
                hash = hash ^ (s[i]);                                                                                                                  
                hash = hash * kPrime;                                                                                          
        }
        return hash;
}


#ifdef TEST
typedef struct ranctx { uint32_t a; uint32_t b; uint32_t c; uint32_t d; } ranctx;
static struct ranctx r0;

#define rot(x,k) (((x)<<(k))|((x)>>(32-(k))))
uint32_t ranval( ranctx *x ) {
    uint32_t e = x->a - rot(x->b, 27);
    x->a = x->b ^ rot(x->c, 17);
    x->b = x->c + x->d;
    x->c = x->d + e;
    x->d = e + x->a;
    return x->d;
}
uint32_t r0gen(void) {
	return ranval(&r0);
}

void raninit( ranctx *x, uint32_t seed ) {
    uint32_t i;
    x->a = 0xf1ea5eed, x->b = x->c = x->d = seed;
    for (i=0; i<20; ++i) {
        (void)ranval(x);
    }
}

void stirfry(unsigned char *s, size_t len) {
	int i;
	for (i = 0; i < len; i++) {
		s[i] = r0gen();
	}
}


int main(int argc, char *argv[]) {
	char *buf;
	int i, j;
	uint64_t k;
	uint64_t N = 10000000000ull;

	raninit(&r0, 1);

	for (i = 8; i < 9; i++) {
		buf = calloc(1, i);

		printf("len =%d\n", i);
		for (k = 0; k < N; k++) {
			stirfry(buf, i);

			if (fnv1a_128(buf, i) == 0) {
				printf("Ok.\n");

				for (i = 0; i < sizeof(buf); i++)
					printf("%02x ", buf[i]);
				printf("\n");
				goto out;
			}
		}
		N += N;

		free(buf);
	}

out:
	return 0;
}
#endif
