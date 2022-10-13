#include <pthread.h>

extern void *cpu(void *ip);
int g_max_ticks = 100;

int main(int argc, char *argv[]) {
	pthread_t thr[4];
	for (int i = 0; i < 4; i++) {
		pthread_create(&thr[i], NULL, cpu, (int) i);
	}
	for (int i = 0; i < 4; i++) {
		pthread_join(thr[i], NULL);
	}
}
