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

.PHONY: check

libleveldb.a:
	$(MAKE) -f Makefile.third_party-leveldb libleveldb.a

ftwdb2: ftwdb2.o $(MD5)
	$(CXX) $(LDFLAGS) -o $@ $^ -lleveldb -lsnappy -lssl -lcrypto

dbtransactor: dbtransactor.o $(MD5)
	$(CXX) $(LDFLAGS) -o $@ $^ -lleveldb -lsnappy -lssl -lcrypto

leveldb_to_bdb:
	$(CXX) -o leveldb_to_bdb leveldb_to_bdb.cc -lleveldb -lsnappy -ldb

bdb_to_leveldb:
	$(CXX) -o bdb_to_leveldb bdb_to_leveldb.cc -lleveldb -lsnappy -ldb

# Misc:
# g++ -lsnappy  dbtransactor.cc -o dbtransactor /usr/lib/x86_64-linux-gnu/libleveldb.a /usr/lib/x86_64-linux-gnu/libsnappy.a -pthread
# g++ -lsnappy  dbmerge.cc -o dbmerge /usr/lib/x86_64-linux-gnu/libleveldb.a /usr/lib/x86_64-linux-gnu/libsnappy.a -pthread
# g++ -lsnappy  dumpdb2.cc -o dumpdb2 /usr/lib/x86_64-linux-gnu/libleveldb.a /usr/lib/x86_64-linux-gnu/libsnappy.a -pthread

# gcc -O2 -c -o md5.o md.c
# g++ -O2 -o ftwdb2_mt ftwdb2_mt.cc worker.cc readn.cc md5.o -lssl -lcrypto -lleveldb -lsnappy -pthread

# Parallel wordcount; RC 2018.
pwc:	pwc.o
	$(CC) $(LDFLAGS) -o $@ $^ -pthread


oldftwdb:
	$(CC) $(LDFLAGS) -o $@ ftwdb_bdb.c

olddumpdb:
	$(CC) $(LDFLAGS) -o $@ dumpdb.c

.PHONY: clean
clean:
	$(MAKE) -f Makefile.third_party-leveldb clean
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
		ftwdb2f \
		ftwdb2g \
		ftwdb2_mt \
		leveldb_to_bdb \
		log2leveldb \
		memback \
		roundup_test \
		sampzero \
		zerorun
