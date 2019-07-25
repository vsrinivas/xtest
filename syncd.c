#include <time.h>
#include <unistd.h>

#define N	32

time_t tms[N];
int i;

main() {
	time_t a, b;
	int n;
	for (;;) {
		sleep(1);

		a = time(NULL);
		sync();
		b = time(NULL);
		tms[i++] = b - a;
		if (i == N - 1) {
			for (n = 0; n < N-1; n++) {
				printf("%d ", tms[n]);
			}
			printf("\n");
			i = 0;
		}
	}
}
