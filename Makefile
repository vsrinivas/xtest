CFLAGS+=-Wall -Wextra -g
OBJS=fftw.o workqueue.o

sftw: libw.a sftw.c $(OBJS)
	$(CC) $(CFLAGS) -g -o sftw sftw.c libw.a -pthread -lrt -I$(HOME)/db.1.85/include $(HOME)/db.1.85/PORT/linux/libdb.a

libw.a: $(OBJS)
	ar rcs libw.a $(OBJS)

dumpdb: 
	$(CC) -g -o dumpdb dumpdb.c -I$(HOME)/db.1.85/include $(HOME)/db.1.85/PORT/linux/libdb.a 

fftw: libw.a
	$(CC) -DTEST -g -o fftw fftw.c workqueue.c libw.a -pthread

clean:
	rm -f $(OBJS) libw.a sftw
