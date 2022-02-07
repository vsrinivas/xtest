#include <pthread.h>
#include "barrier.h"

void bwait(struct barrier *b) {
	pthread_mutex_lock(&b->b_mtx);
	while (b->b_count > 0)
		pthread_cond_wait(&b->b_cv, &b->b_mtx);
        pthread_mutex_unlock(&b->b_mtx);
}

void binc(struct barrier *b) {
        pthread_mutex_lock(&b->b_mtx);
        b->b_count++;
        pthread_mutex_unlock(&b->b_mtx);
}

int bdec(struct barrier *b) {
	int ret = 0;
        pthread_mutex_lock(&b->b_mtx);
        if (--b->b_count == 0) {
		ret = b->b_count;
                pthread_cond_signal(&b->b_cv);
	}
	pthread_mutex_unlock(&b->b_mtx);
	return ret;
}
