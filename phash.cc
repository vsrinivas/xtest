#include <pthread.h>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

std::vector<char*> files;
pthread_mutex_t mtx;
pthread_cond_t wait_;
int running;

#define NPROC (2)

void *thr(void *arg) {
	char buf[180];
	for (;;) {
		pthread_mutex_lock(&mtx);
		if (files.empty()) {
			running--;
			pthread_cond_signal(&wait_);
			pthread_mutex_unlock(&mtx);
			return 0;
		}
		char* file = files.back();
		files.pop_back();
		sprintf(buf, "gsutil hash -m %s", file);
		pthread_mutex_unlock(&mtx);
		system(buf);
	}
}

int main(int argc, char *argv[]) {
	for (int i = 1; i < argc; i++) {
		files.push_back(strdup(argv[i]));
	}

	running = NPROC;
	for (int i = 0; i < NPROC; i++) {
		pthread_t t;
		pthread_create(&t, 0, thr, 0);
	}

	for (;;) {
		pthread_mutex_lock(&mtx);
		if (running == 0)
			return 0;
		pthread_cond_wait(&wait_, &mtx);
		pthread_mutex_unlock(&mtx);
	}	
}
