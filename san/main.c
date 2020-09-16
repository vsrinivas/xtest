#include <pthread.h>

extern void *cpu(void *ip);

int main(int argc, char *argv[]) {
	for (int i = 0; i < 4; i++) {
		pthread_t thr;
		pthread_create(&thr, NULL, cpu, (int) i);
	}
	pause();
}
