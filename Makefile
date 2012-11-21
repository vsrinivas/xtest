CFLAGS+=-Wall -Wextra
OBJS=fftw.o workqueue.o

sftw: libw.a sftw.c $(OBJS)
	$(CC) -g -o sftw sftw.c libw.a -pthread -lrt 

libw.a: $(OBJS)
	ar rcs libw.a $(OBJS)
clean:
	rm -f $(OBJS) libw.a sftw
