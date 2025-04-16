#include <sys/mman.h>
#include <stddef.h>

#define KB (1024ul)
#define MB (1024 * KB)
#define GB (1024 * MB)

int main(int argc, char *argv[]) {
	char *b = mmap(NULL, GB, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_SHARED, -1, 0);
	int i;

	for (i = 0; i < GB; i++) {
		b[i] = 0xf4; /* HLT */
	}
	b[1097] = 0xc3;	/* RET */

	((void (*)(void)) &b[1097])();
}
