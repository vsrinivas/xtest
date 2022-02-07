#include <sys/queue.h>

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <pthread.h>

#include "workqueue.h"

/* Max workqueues (and max workqueue threads) */
#define SCHED_WORKQUEUES (4)

/* Maximum number of workers; currently equal to max workqueues */
static const int sched_max_proc = SCHED_WORKQUEUES;

struct workqueue_item {
	void				(*wi_cb)(void *, struct barrier *);
	void				*wi_priv;
	struct barrier			*wi_barrier;
	TAILQ_ENTRY(workqueue_item)	wi_entries;
};

struct workqueue {
	pthread_t			wq_td;
	pthread_mutex_t			wq_mtx;
	pthread_cond_t			wq_cv;
	TAILQ_HEAD(, workqueue_item)	wq_entries;
	int				wq_items;
	int				wq_should_exit;

	int				wq_processed;
	int				wq_pulls;
};

static pthread_once_t workqueue_once = PTHREAD_ONCE_INIT;
static struct workqueue workqueues[SCHED_WORKQUEUES];
static int nproc = 0;

static void process_workqueue_item(struct workqueue_item *wi) {
	wi->wi_cb(wi->wi_priv, wi->wi_barrier);
	if (wi->wi_barrier)
		bdec(wi->wi_barrier);
	free(wi);
}

/*
 * Workqueue is locked on entry and unlocked around processing each item.
 */
static int process_workqueue(struct workqueue *wq, int num) {
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
		pthread_mutex_unlock(&wq->wq_mtx);
		process_workqueue_item(wi);
		pthread_mutex_lock(&wq->wq_mtx);
		wq->wq_processed++;
	}

	return (i);
}

static struct workqueue *selqueue(void) {
	struct workqueue *wq;
	static int rover = 0;

	wq = &workqueues[rover++ % nproc];

	return (wq);
}

static int add_workqueue_item(struct workqueue_item *wi) {
	struct workqueue *wq;

	if (wi->wi_barrier)
		binc(wi->wi_barrier);

	wq = selqueue();
	pthread_mutex_lock(&wq->wq_mtx);

	TAILQ_INSERT_TAIL(&wq->wq_entries, wi, wi_entries);
	wq->wq_items++;

	pthread_cond_signal(&wq->wq_cv);
	pthread_mutex_unlock(&wq->wq_mtx);

	return (0);
}

static void *workqueue_thread(void *p) {
	struct workqueue *wq, *rwq;
	int myidx;
	int i;

	wq = (struct workqueue *) p;

	for (;;) {
		pthread_mutex_lock(&wq->wq_mtx);
		if (wq->wq_items == 0) {
			if (wq->wq_should_exit) {
				pthread_mutex_unlock(&wq->wq_mtx);
				break;
			} else {
				pthread_cond_wait(&wq->wq_cv, &wq->wq_mtx);
			}
		}

		/*
		 * Process all items on our workqueue, in-order
		 */
		if (wq->wq_items)
			process_workqueue(wq, -1);

		pthread_mutex_unlock(&wq->wq_mtx);
	}

	return (NULL);
}

static void workqueue_exit(void) {
	struct workqueue *wq;
	struct barrier *completion;
	int i;

	for (i = 0; i < nproc; i++) {
		wq = &workqueues[i];

		pthread_mutex_lock(&wq->wq_mtx);
		wq->wq_should_exit = 1;
		pthread_cond_signal(&wq->wq_cv);
		pthread_mutex_unlock(&wq->wq_mtx);
		pthread_join(wq->wq_td, NULL);
	}
}

static void workqueue_init(void) {
	char *numproc;
	int i;

	nproc = SCHED_WORKQUEUES;

	do {
		numproc = getenv("NUMPROC");
		if (numproc == NULL)
			break;
		i = atoi(numproc);
		if (i < 1)
			i = 1;
		if (i > sched_max_proc)
			i = sched_max_proc;
		nproc = i;
	} while(0);

	for (i = 0; i < nproc; i++) {
		workqueues[i].wq_items = 0;
		TAILQ_INIT(&workqueues[i].wq_entries);
		pthread_mutex_init(&workqueues[i].wq_mtx, NULL);
		workqueues[i].wq_processed = 0;
		workqueues[i].wq_pulls = 0;
		pthread_create(&workqueues[i].wq_td, NULL, &workqueue_thread, &workqueues[i]);
	}

	atexit(workqueue_exit);
}

int
workqueue(void (*fn)(void *priv, struct barrier *b), void *priv,
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
