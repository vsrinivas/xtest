#ifndef _FFTW_H_
#define _FFTW_H_

extern int
fftw(const char *path,
     int (*fn)(const char *, const struct stat *, int, void *),
     void *priv);

#endif

