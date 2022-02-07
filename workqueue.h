#ifndef _WORKQUEUE_H_
#define _WORKQUEUE_H_

#ifdef __cplusplus
extern "C" {
#endif

struct barrier {
        pthread_mutex_t b_mtx;
        pthread_cond_t  b_cv;
        int             b_count;
};

void binc(struct barrier *b);
int bdec(struct barrier *b);
void bwait(struct barrier *b);

int workqueue(void (*fn)(void *priv, struct barrier *), void *priv,
	      struct barrier *b); 

#ifdef __cplusplus
};
#endif

#endif
