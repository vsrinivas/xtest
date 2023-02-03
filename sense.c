#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>

/* Synchronization primitive - Sense-reversed asymmetric barrier w/ closure */
/*
 * |barrier| and await/bwait/bgo/bclose implement a Sense-reversed barrier
 * with a couple of twists - bwait() waits for all await()s, but does not
 * release await() - bgo() does that. This allows one thread to bwait() for
 * all workers to arrive at the barrier, do some work to aggregate and
 * prepare for a next phase, and then release the workers.
 *
 * This barrier also supports |bclose|, which causes new await()s to return
 * immediately with an error. This allows closing down a problem cleanly.
 *
 * See The Art of Multiprocessor Programming (Herlihy & Shavit), first ed.,
 *   section 17.3 for discussion of a sense-reversing barrier.
 *
 * The split between await() and bwait() and support for closure is non
 * standard and I have not seen it in literature, but it is pretty useful.
 */
struct barrier {
        int max;
        int val;
        bool sense;
        bool closed;
        pthread_mutex_t mu;
        pthread_cond_t cv;
};

bool await(struct barrier *bp, bool *sense) {
        pthread_mutex_lock(&bp->mu);
        if (bp->closed) {
                pthread_mutex_unlock(&bp->mu);
                return true;
        }
        int val = bp->val--;
        if (val == 1)
                pthread_cond_broadcast(&bp->cv);

        while (bp->sense != *sense) {
                pthread_cond_wait(&bp->cv, &bp->mu);
                if (bp->closed) {
                        pthread_mutex_unlock(&bp->mu);
                        return false;
                }
        }

        *sense = !(*sense);
        pthread_mutex_unlock(&bp->mu);
        return false;
}

void binit(struct barrier *bp, int max) {
        bp->max = max;
        bp->val = max;
        bp->sense = true;
        bp->closed = false;
}

void bwait(struct barrier *bp) {
        pthread_mutex_lock(&bp->mu);
        while (bp->val != 0)
                pthread_cond_wait(&bp->cv, &bp->mu);
        pthread_mutex_unlock(&bp->mu);
}

void bgo(struct barrier *bp, bool *sense) {
        pthread_mutex_lock(&bp->mu);
        bp->val = bp->max;
        bp->sense = *sense;
        *sense = !(*sense);
        pthread_cond_broadcast(&bp->cv);
        pthread_mutex_unlock(&bp->mu);
}


void bclose(struct barrier *bp) {
        pthread_mutex_lock(&bp->mu);
        bp->closed = true;
        pthread_cond_broadcast(&bp->cv);
        pthread_mutex_unlock(&bp->mu);
}

/* -------------------- CUT ON THE DOTTED LINE *************/
