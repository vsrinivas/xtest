#ifndef _WORKQUEUE_H_
#define _WORKQUEUE_H_

struct barrier {
        pthread_mutex_t b_mtx;
        pthread_cond_t  b_cv;
        int             b_count;
};

void bwait(struct barrier *b);

int workqueue(void (*fn)(void *priv, struct barrier *), void *priv,
	      struct barrier *b); 

#endif
