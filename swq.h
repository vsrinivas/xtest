#ifndef _WORKQUEUE_H_
#define _WORKQUEUE_H_

void swq(void (*fn)(void *priv), void *priv);

#endif
