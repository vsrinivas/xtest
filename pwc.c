#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/param.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <strings.h>
#include <fcntl.h>
#include <ftw.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#include <stdatomic.h>

atomic_bool should_exit_;

struct file {
	const char *path;
	int fd;
	char *mapping;
	uint64_t size;
	uint64_t lines;
};

/* FIFO for posted work */
#define FIFO_SIZE	(512)
static atomic_int fifo_head_;
static atomic_int fifo_done_;
static struct file fifo_[FIFO_SIZE];

/* FIFO for completed work */
static atomic_int wfifo_head_;
static atomic_int wfifo_done_;
static struct file wfifo_[FIFO_SIZE];

uint64_t lines(struct file *fp) {
	uint64_t ctr = 0;
	uint64_t len = fp->size;
	char *base = fp->mapping;

	for (;;) {
		base = memchr(base, '\n', len - (base - fp->mapping));
		if (!base)
			break;

		ctr++;
		base = base+1;
	}

	return ctr;
}

void *iothread(void *__unused) {
	struct file *f;
	int fifo_tail = 0;

	for (;;) {
		bool should_exit = atomic_load_explicit(&should_exit_,
							memory_order_acquire);

		if (atomic_load_explicit(&fifo_head_, memory_order_acquire) != fifo_tail) {
			while (atomic_load_explicit(&fifo_head_, memory_order_acquire) != fifo_tail) {
				f = &fifo_[fifo_tail++ & (FIFO_SIZE - 1)];
				/* Count newlines in f; push to completion FIFO. */
				f->lines = lines(f);
				wpush(*f);
			}
			atomic_store_explicit(&fifo_done_, fifo_tail, memory_order_release);
		}
		if (should_exit)
			break;
	}
	return NULL;
}

/* Push to work fifo */
void push(struct file f) {
	while (atomic_load_explicit(&fifo_head_, memory_order_acquire) -
	       atomic_load_explicit(&fifo_done_, memory_order_acquire) == (FIFO_SIZE - 1))
		;

	int n = atomic_load_explicit(&fifo_head_, memory_order_acquire) + 1;
	fifo_[n & (FIFO_SIZE - 1)] = f;
	atomic_store_explicit(&fifo_head_, n, memory_order_release);
}

/* Push to done fifo */  
void wpush(struct file f) {
        while (atomic_load_explicit(&wfifo_head_, memory_order_acquire) -
               atomic_load_explicit(&wfifo_done_, memory_order_acquire) == (FIFO_SIZE - 1))
                ;

	int n = atomic_load_explicit(&wfifo_head_, memory_order_acquire) + 1;
	wfifo_[n & (FIFO_SIZE - 1)] = f;
	atomic_store_explicit(&wfifo_head_, n, memory_order_release);
}

int wdone() {
        return (atomic_load_explicit(&wfifo_head_, memory_order_acquire) -
                atomic_load_explicit(&wfifo_done_, memory_order_acquire));
}

void reap_done(void) {
	static int wfifo_tail;
	struct file *fp;

	if (atomic_load_explicit(&wfifo_head_, memory_order_acquire) != wfifo_tail) {
		while (atomic_load_explicit(&wfifo_head_, memory_order_acquire) != wfifo_tail) {
			fp = &wfifo_[wfifo_tail++ & (FIFO_SIZE - 1)];
			printf("%s %ld\n", fp->path, fp->lines);
			free(fp->path);
			munmap(fp->mapping, fp->size);
			close(fp->fd);
		}
		atomic_store_explicit(&wfifo_done_, wfifo_tail, memory_order_release);
	}
}

int file_cb(const char *fpath,
	    const struct stat *sb,
	    int typeflag,
	    struct FTW *ftwbuf) {
	struct file f;

	if (typeflag == FTW_F) {
		f.path = strdup(fpath);
		f.fd = open(fpath, O_RDONLY);
		if (f.fd == -1) {
			printf("open: %d\n", errno);
			pause();
		}
		f.size = sb->st_size;
		f.mapping = mmap(NULL, roundup(sb->st_size, 4096), PROT_READ,
				 MAP_PRIVATE | MAP_FILE	| MAP_POPULATE,
				 f.fd, 0);
		posix_fadvise(f.fd, 0, sb->st_size, POSIX_FADV_WILLNEED);
		push(f);
	}

	if (wdone() > (FIFO_SIZE / 2))
		reap_done();

	
	return 0;
}

int main(int argc, char *argv[]) {
	int rc;
	pthread_t io;
	
	pthread_create(&io, NULL, iothread, NULL);
	rc = ftw(argv[1], file_cb, FTW_DEPTH);

	atomic_store_explicit(&should_exit_, true, memory_order_release); 
	pthread_join(io, NULL);
	reap_done();

	return 0;
}
