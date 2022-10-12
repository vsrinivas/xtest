#ifndef DEFER_H_
#define DEFER_H_

#include <stddef.h>

void defer_init(void);
void defer_munmap(int fd, void *buf, size_t);
void defer_close(int fd);
void defer_sync(void);

#endif
