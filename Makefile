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
	dbtransactor \
	pwc \
	while \
	whilehammer

.PHONY: default
default: $(CMDS)

libleveldb.a:
	$(MAKE) -f Makefile.third_party-leveldb libleveldb.a
libninja-test.a:
	$(MAKE) -f Makefile.third_party-ninja-test libninja-test.a

fnv1a.o: fnv1a.c

fnv1a_test: fnv1a_test.o fnv1a.o libninja-test.a
	$(CXX) $(LDFLAGS) -o fnv1a_test $^

ftwdb2: ftwdb2.o $(MD5)
	$(CXX) $(LDFLAGS) -o $@ $^ -lleveldb -lsnappy -lssl -lcrypto

dbtransactor: dbtransactor.o $(MD5)
	$(CXX) $(LDFLAGS) -o $@ $^ -lleveldb -lsnappy -lssl -lcrypto

leveldb_to_bdb:
	$(CXX) -o leveldb_to_bdb leveldb_to_bdb.cc -lleveldb -lsnappy -ldb

bdb_to_leveldb:
	$(CXX) -o bdb_to_leveldb bdb_to_leveldb.cc -lleveldb -lsnappy -ldb

# Parallel wordcount; RC 2018.
pwc:	pwc.o
	$(CC) $(LDFLAGS) -o $@ $^ -pthread

oldftwdb:
	$(CC) $(LDFLAGS) -o $@ ftwdb_bdb.c

olddumpdb:
	$(CC) $(LDFLAGS) -o $@ dumpdb.c

.PHONY: check
check: fnv1a_test
	./fnv1a_test

.PHONY: clean
clean:
	$(MAKE) -f Makefile.third_party-leveldb clean
	$(MAKE) -f Makefile.third_party-ninja-test clean
	rm -f $(UTIL) $(CMDS) ftwdb2.o transactor.o pwc.o oldftwdb olddumpdb
	rm -f \
		bdb_to_leveldb \
		countcphash \
		countcphash2 \
		cpcr2 \
		cphash \
		dbtransactor \
		dumpdb \
		dumpdb2 \
		ftw2db2log \
		leveldb_to_bdb \
		log2leveldb \
		memback \
		roundup_test \
		sampzero \
		zerorun \
		fnv1a_test \
		fnv1a.o
