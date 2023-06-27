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
	\
	cpu-check-medium.exe \
	cpu-check-small.exe \
	cpu-check-serial.exe

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

leveldb_to_bdb:
	$(CXX) -o leveldb_to_bdb leveldb_to_bdb.cc -lleveldb -lsnappy -ldb

bdb_to_leveldb:
	$(CXX) -o bdb_to_leveldb bdb_to_leveldb.cc -lleveldb -lsnappy -ldb

cpu-check-medium.exe: hashes.o murmur3.o zencpy.o
	$(CXX) -o cpu-check-medium.exe cpu-check.cc hashes.o murmur3.o zencpy.o -DN=4294967296UL  -O2 -DDEBUG -g -march=native -DPARALLEL -pthread

cpu-check-small.exe: hashes.o murmur3.o zencpy.o
	$(CXX) -o cpu-check-small.exe cpu-check.cc hashes.o murmur3.o zencpy.o -DN=1048576UL  -O2 -DDEBUG -g -march=native -DPARALLEL -pthread

cpu-check-serial.exe: hashes.o murmur3.o zencpy.o
	$(CXX) -o cpu-check-serial.exe cpu-check.cc hashes.o murmur3.o zencpy.o -DN=1048576UL  -O2 -DDEBUG -g -march=native


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

.PHONY: check
check: hashes_test pwq_test cpuid $(CMDS)
	./hashes_test
	./pwq_test
	./cpuid
	./cpu-check-small.exe 65536 16
	$(MAKE) -C san check

.PHONY: clean
clean:
	$(MAKE) -f Makefile.third_party-leveldb clean
	$(MAKE) -f Makefile.third_party-ninja-test clean
	rm -f $(UTIL) $(CMDS) ftwdb2.o transactor.o pwc.o oldftwdb olddumpdb
