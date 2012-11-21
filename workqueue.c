#include <sys/param.h>
#include <sys/queue.h>

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "workqueue.h"

	/* Max workqueues (and max workqueue threads) */
#define SCHED_WORKQUEUES (4)
	/* Default workqueue threads, if NUMPROC is unset */
static const int sched_defthreads = 4;
	/* Default ticks before we pull workitems from a remote workqueue */
static const int sched_sticks_rr = 4; 
	/* Workitems to pull from a remote workqueue */
static const int sched_rr_pull = 2;
static const int sched_workqueues = SCHED_WORKQUEUES;
static const int sched_max_proc = SCHED_WORKQUEUES;
	/* If workqueue has over this amount of work, do work in workqueue() */
static const int sched_overload_threshold = 4;
	/* Max items to process in process_workqueue_item */
static int sched_overload = 2;

int sched_forceproc = -1;
int sched_forceoverload = -1;

struct workqueue_item {
	void				(*wi_cb)(void *, struct barrier *);
	void				*wi_priv;
	struct barrier			*wi_barrier;
	TAILQ_ENTRY(workqueue_item)	wi_entries;
};

struct workqueue {
	pthread_mutex_t			wq_mtx;
	pthread_cond_t			wq_cv;
	TAILQ_HEAD(, workqueue_item)	wq_entries;
	int				wq_items;
	int				wq_processed;
};

static pthread_mutex_t sched_mtx;
static pthread_cond_t sched_cv;
static pthread_once_t workqueue_once = PTHREAD_ONCE_INIT;
static struct workqueue workqueues[SCHED_WORKQUEUES];
static int workqueue_rover = 0;

void bwait(struct barrier *b) {
	pthread_mutex_lock(&b->b_mtx);
	while (b->b_count > 0)
		pthread_cond_wait(&b->b_cv, &b->b_mtx);
        pthread_mutex_unlock(&b->b_mtx);
}

static void binc(struct barrier *b) {
        pthread_mutex_lock(&b->b_mtx);
        b->b_count++;
        pthread_mutex_unlock(&b->b_mtx);
}

static void bdec(struct barrier *b) {
        pthread_mutex_lock(&b->b_mtx);
        if (--b->b_count == 0)
                pthread_cond_signal(&b->b_cv);
	pthread_mutex_unlock(&b->b_mtx);
}

static void process_workqueue_item(struct workqueue_item *wi) {
	wi->wi_cb(wi->wi_priv, wi->wi_barrier);
	if (wi->wi_barrier)
		bdec(wi->wi_barrier);
	free(wi);
}

/*
 * Workqueue is locked on entry and unlocked around processing each item.
 */
static void process_workqueue(struct workqueue *wq, int num) {
	struct workqueue_item *wi;
	int i;

	for (i = 0;; i++) {
		if (i == num)
			break;

		wi = TAILQ_FIRST(&wq->wq_entries);
		if (wi == NULL)
			break;

		TAILQ_REMOVE(&wq->wq_entries, wi, wi_entries);
		wq->wq_items--;
		wq->wq_processed++;
		pthread_mutex_unlock(&wq->wq_mtx);
		process_workqueue_item(wi);
		pthread_mutex_lock(&wq->wq_mtx);
	}
}

static int add_workqueue_item(struct workqueue_item *wi) {
	static int rover = 0;
	int selqueue;
	struct workqueue *wq;

	if (wi->wi_barrier)
		binc(wi->wi_barrier);

	selqueue = (rover++) & (workqueue_rover - 1);
	wq = &workqueues[selqueue];
	pthread_mutex_lock(&wq->wq_mtx);

	TAILQ_INSERT_TAIL(&wq->wq_entries, wi, wi_entries);
	wq->wq_items++;

	if ((wq->wq_items > sched_overload_threshold) &&
	    (sched_overload != 0))
		process_workqueue(wq, sched_overload);

	pthread_cond_signal(&wq->wq_cv);
	pthread_mutex_unlock(&wq->wq_mtx);

	return (0);
}

static void *workqueue_thread(void *p) {
	struct workqueue *wq, *rwq;
	int myidx;
	int rover;
	int sticks;
	int i;

	pthread_mutex_lock(&sched_mtx);
	myidx = workqueue_rover++ % sched_workqueues;
	rover = myidx;
	wq = &workqueues[myidx];

	TAILQ_INIT(&wq->wq_entries);
	pthread_mutex_init(&wq->wq_mtx, NULL);
	wq->wq_items = 0;

	pthread_cond_signal(&sched_cv);
	pthread_mutex_unlock(&sched_mtx);

	for (sticks = 0;; sticks++) {
		pthread_mutex_lock(&wq->wq_mtx);
		if (wq->wq_items == 0)
			pthread_cond_wait(&wq->wq_cv, &wq->wq_mtx);

		/*
		 * Process all items on our workqueue
		 */
		if (wq->wq_items)
			process_workqueue(wq, -1);

		pthread_mutex_unlock(&wq->wq_mtx);

		/*
		 * Once every sched_sticks_rr ticks, pull one workitem from
		 * a remote workqueue.
		 */
		 if ((workqueue_rover > 1) &&
		     (sticks % sched_sticks_rr == 0)) {
			rover = (rover + 1) % workqueue_rover;
			if (rover == myidx)
				continue;

			rwq = &workqueues[rover];

			i = pthread_mutex_trylock(&rwq->wq_mtx);
			if (i != 0)
				continue;
			process_workqueue(rwq, sched_rr_pull);
			pthread_mutex_unlock(&rwq->wq_mtx);
		}

	}
	/*NOTREACHED*/
	return (NULL);
}

static void workqueue_init(void)
{
	pthread_t thr;
	char *numproc;
	int nproc;
	int i;

	nproc = sched_defthreads;

	do {
		if (sched_forceproc != -1) {
			nproc = sched_forceproc;
			break;
		}

		numproc = getenv("NUMPROC");
		if (numproc == NULL)
			break;
		i = atoi(numproc);
		if (i < 1)
			i = 1;
		nproc = MIN(i, sched_max_proc);
	} while(0);


	for (i = 0; i < SCHED_WORKQUEUES; i++)
		TAILQ_INIT(&workqueues[i].wq_entries);

	if (sched_forceoverload != -1)
		sched_overload = sched_forceoverload;

	pthread_mutex_lock(&sched_mtx);
	for (i = 0; i < nproc; i++) {
		pthread_create(&thr, NULL, &workqueue_thread, NULL);
		pthread_cond_wait(&sched_cv, &sched_mtx);
	}
	pthread_mutex_unlock(&sched_mtx);
}

int workqueue(void (*fn)(void *priv, struct barrier *b), void *priv,
	      struct barrier *b) 
{
	struct workqueue_item *it;
	int i;

	pthread_once(&workqueue_once, workqueue_init);

	it = malloc(sizeof(struct workqueue_item));
	it->wi_cb = fn;
	it->wi_priv = priv;
	it->wi_barrier = b;
	i = add_workqueue_item(it);
	if (i == -1)
		free(it);

	return (i);
}

