OBJS=fftw.o workqueue.o

sftw: libw.a
	cc -g -o sftw sftw.c libw.a -pthread -lrt

libw.a: $(OBJS)
	ar rcs libw.a $(OBJS)

clean:
	rm -f $(OBJS) libw.a
