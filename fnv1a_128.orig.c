#include <sys/types.h>
#include <stdint.h>

static char *format128(unsigned __int128 v) {
	static char buf[33] = {};
	int i;

	sprintf(buf, "%lx", v >> 64);
	sprintf(buf + 16, "%lx", v);
	return buf;
}
 
unsigned __int128 fnv1_128(unsigned char *s, size_t len) {
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
                hash = hash * kPrime;                                                                                                         
                hash = hash ^ (s[i]);                                                                                                                  
        }
        return hash;
}


#ifdef TEST

int main(int argc, char *argv[]) {
	unsigned char buf[] = { 0x20, 0x28, 0x4e, 0x43, 0x40, 0x55, 0x6f, 0x99, 0x25, 0x1b, 0x89, 0xf4, 0xa8, 0x18, 0xec, 0x76, 0xc0 };
	char *buf;
	int i, j, k;
	int N;


	for (i = 8; i < 32; i++) {
		buf = calloc(1, i);

		printf("len =%d\n", i);
		for (k = 0; k < N; k++) {

		}

		free(buf);
	}


	if (fnv1_128(buf, sizeof(buf)) == 0) {
		printf("Ok.\n");
	
		for (i = 0; i < sizeof(buf); i++)
			printf("%02x ", buf[i]);
		printf("\n");	
	}
}
#endif
