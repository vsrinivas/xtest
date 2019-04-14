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

# CFLAGS=-static CXXFLAGS="-O -static" LDFLAGS="-static -pthread -lsnappy" make ftwdb2
ftwdb2: ftwdb2.o $(MD5)
	$(CXX) $(LDFLAGS) -o $@ $^ -lleveldb -lsnappy -lssl -lcrypto

# CFLAGS=-static CXXFLAGS="-O -static" LDFLAGS="-static -pthread -lsnappy" make transactor
transactor: transactor.o $(MD5) $(THREADS)
	$(CXX) $(LDFLAGS) -o $@ $^ -pthread -lleveldb -lsnappy

# Misc:
# g++ -lsnappy  dbtransactor.cc -o dbtransactor /usr/lib/x86_64-linux-gnu/libleveldb.a /usr/lib/x86_64-linux-gnu/libsnappy.a -pthread
# g++ -lsnappy  dbmerge.cc -o dbmerge /usr/lib/x86_64-linux-gnu/libleveldb.a /usr/lib/x86_64-linux-gnu/libsnappy.a -pthread
# g++ -lsnappy  dumpdb2.cc -o dumpdb2 /usr/lib/x86_64-linux-gnu/libleveldb.a /usr/lib/x86_64-linux-gnu/libsnappy.a -pthread

# gcc -O2 -c -o md5.o md.c
# g++ -O2 -o ftwdb2_mt ftwdb2_mt.cc worker.cc readn.cc md5.o -lssl -lcrypto -lleveldb -lsnappy -pthread

# Parallel wordcount; RC 2018.
pwc:	pwc.o
	$(CC) $(LDFLAGS) -o $@ $^ -pthread

.PHONY: clean
clean:
	rm -f $(UTIL) $(CMDS) ftwdb2.o transactor.o pwc.o
