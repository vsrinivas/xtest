#ifndef _WORKQUEUE_H_
#define _WORKQUEUE_H_

#include "barrier.h"

#ifdef __cplusplus
extern "C" {
#endif

int workqueue(void (*fn)(void *priv, struct barrier *, uint64_t), void *priv,
	      struct barrier *b, uint64_t key);

#ifdef __cplusplus
};
#endif

#endif
