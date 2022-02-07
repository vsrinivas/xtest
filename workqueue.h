#ifndef _WORKQUEUE_H_
#define _WORKQUEUE_H_

#include "barrier.h"

#ifdef __cplusplus
extern "C" {
#endif

int workqueue(void (*fn)(void *priv, struct barrier *), void *priv,
	      struct barrier *b); 

#ifdef __cplusplus
};
#endif

#endif
