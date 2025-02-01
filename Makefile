MD5=\
	md5.o

THREADS=\
	barrier.o \
	pwq.o \
	workqueue.o

OLD=\
	fftw.o

UTIL=\
	$(MD5) \
	$(THREADS)

CMDS=\
	cpuid \
	cphash \
	countcphash \
	ftwdb2 \
	ftwdb2_fnv \
	dbmerge \
	dbtransactor \
	pwc \
	while \
	whilehammer \
	xwait \

.PHONY: default
default: $(CMDS)

libleveldb.a:
	$(MAKE) -f Makefile.third_party-leveldb libleveldb.a
libninja-test.a:
	$(MAKE) -f Makefile.third_party-ninja-test libninja-test.a

ftwdb2: ftwdb2.o $(MD5)
	$(CXX) $(LDFLAGS) -o $@ $^ -lleveldb -lsnappy -lssl -lcrypto

ftwdb2_fnv: ftwdb2_fnv.o hashes.o
	$(CXX) $(LDFLAGS) -o $@ $^ -lleveldb -lsnappy

dbmerge: dbmerge.o
	$(CXX) $(LDFLAGS) -o $@ $^ -lleveldb -lsnappy -lssl -lcrypto

dbtransactor: dbtransactor.o
	$(CXX) $(LDFLAGS) -o $@ $^ -lleveldb -lsnappy -lssl -lcrypto

dumpdb2: dumpdb2.o
	$(CXX) $(LDFLAGS) -o $@ $^ -lleveldb -lsnappy -lssl -lcrypto

leveldb_to_bdb:
	$(CXX) -o leveldb_to_bdb leveldb_to_bdb.cc -lleveldb -lsnappy -ldb

bdb_to_leveldb:
	$(CXX) -o bdb_to_leveldb bdb_to_leveldb.cc -lleveldb -lsnappy -ldb

pcpucheck: pcpucheck.o vcopy.o vcopy2.o zencpy.o zencpy2.o murmur3.o hashes.o
	$(CXX) -o pcpucheck $^

# Parallel wordcount; RC 2018.
pwc:	pwc.o
	$(CC) $(LDFLAGS) -o $@ $^ -pthread

oldftwdb:
	$(CC) $(LDFLAGS) -o $@ ftwdb_bdb.c

olddumpdb:
	$(CC) $(LDFLAGS) -o $@ dumpdb.c

countcphash: countcphash.o hashes.o
	$(CXX) $(LDFLAGS) -o $@ $^

hashes_test: hashes.o hashes_test.o
	$(CC) $(LDFLAGS) -o $@ hashes_test.o hashes.o

pwq_test: pwq_test.o $(THREADS)
	$(CXX) $(LDFLAGS) -o $@ pwq_test.o $(THREADS) -pthread

memcpy_test: memcpy_test.o vcopy.o vcopy2.o zencpy.o zencpy2.o

.PHONY: check
check: hashes_test pwq_test cpuid memcpy_test $(CMDS)
	./hashes_test
	./pwq_test
	./cpuid
	./memcpy_test
	./pcpucheck 65536 16
	$(MAKE) -C san check

.PHONY: clean
clean:
	$(MAKE) -f Makefile.third_party-leveldb clean
	$(MAKE) -f Makefile.third_party-ninja-test clean
	rm -f $(UTIL) $(CMDS) ftwdb2.o transactor.o pwc.o oldftwdb olddumpdb
