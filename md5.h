#ifndef _MD5_H_
#define _MD5_H_

/* MD5 a file; return 32-byte string of 128-bit hash */
/* Caller is responsible for deallocating return value w/ ::free */
char* md5sum(char* path);

#endif  // _MD5_H_
