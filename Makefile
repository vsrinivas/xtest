MD5=\
	md5.o

THREADS=\
	barrier.o \
	pwq.o \
	fftw.o

UTIL=\
	$(MD5) \
	$(THREADS)

CMDS=\
	ftwdb2 \
	transactor

ftwdb2: ftwdb2.o $(MD5)
	$(CXX) $(LDFLAGS) -o $@ $+ -lleveldb

transactor: transactor.o

.PHONY: clean
clean:
	rm -f $(UTIL) $(CMDS) ftwdb2.o transactor.o
