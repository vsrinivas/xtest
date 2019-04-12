MD5=\
	md5.o

THREADS=\
	barrier.o \
	pwq.o \

OLD=\
	fftw.o

UTIL=\
	$(MD5) \
	$(THREADS)

CMDS=\
	ftwdb2 \
	transactor \
	pwc

ftwdb2: ftwdb2.o $(MD5)
	$(CXX) $(LDFLAGS) -o $@ $^ -lleveldb

transactor: transactor.o $(MD5) $(THREADS)
	$(CXX) $(LDFLAGS) -o $@ $^ -pthread

# Parallel wordcount; RC 2018.
pwc:	pwc.o
	$(CC) $(LDFLAGS) -o $@ $^ -pthread

.PHONY: clean
clean:
	rm -f $(UTIL) $(CMDS) ftwdb2.o transactor.o pwc.o
