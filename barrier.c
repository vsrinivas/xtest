#include <pthread.h>
#include <sys/select.h>
struct barrier {
	int fd[2];
	int count;
	pthread_mutex_t mtx;
};

int binit(struct barrier *b) {
	pipe(&b->fd, 0);
	b->count = 0;
	return b->fd[0];
}

void bwait(struct barrier *b, int f) {
	int v;
	fd_set set;

	for (;;) {
		FD_ZERO(&set);
		FD_SET(f, &set);
		select(f+1, &set, NULL, NULL, NULL);
		if (FD_ISSET(f, &set))
			if (bval(b) == 0)
				break;
	}
	close(b->fd[0]);
	close(b->fd[1]);
}

int bval(struct barrier *b) {
	int val;
	pthread_mutex_lock(&b->mtx);
	val = b->count;
	pthread_mutex_unlock(&b->mtx);
	return val;
}

static void binc(struct barrier *b) {
	pthread_mutex_lock(&b->mtx);
	++b->count;
	pthread_mutex_unlock(&b->mtx);
}

static void bdec(struct barrier *b) {
	int sig = -1;
	char c = 0;
	pthread_mutex_lock(&b->mtx);
	if (--b->count == 0)
		sig = b->fd[1];
	pthread_mutex_unlock(&b->mtx);
	if (sig > -1)
		write(sig, &c, 1);
}


main() {
	struct barrier b;
	int x = binit(&b);

	switch (fork()) {
	case 0:
		binc(&b);
		sleep(10);
		bdec(&b);
		exit(0);
	default:
		bwait(&b,x);
		printf("Hi\n");
	}
}
