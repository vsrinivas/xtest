#ifndef _MD4_H_
#define _MD4_H_

#include <stddef.h>

/* MD4 a file; return 32-byte string of 128-bit hash */
/* Caller is responsible for deallocating return value w/ ::free */
#ifdef __cplusplus
extern "C"
#endif
char *md4sum(const char *path);

#ifdef __cplusplus
extern "C"
#endif
char *md4sumbuf(const char *buf, size_t size);

#endif  // _MD4_H_
