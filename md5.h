#ifndef _MD5_H_
#define _MD5_H_

#include <stddef.h>

/* MD5 a file; return 32-byte string of 128-bit hash */
/* Caller is responsible for deallocating return value w/ ::free */
#ifdef __cplusplus
extern "C"
#endif
char *md5sum(const char *path);

#ifdef __cplusplus
extern "C"
#endif
char *md5sumbuf(const char *buf, size_t size);

#endif  // _MD5_H_
