#include <sys/types.h>
#include <sys/stat.h>

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <fcntl.h>
#include <db.h>

#include <pthread.h>

#include <cassert>
#include <vector>
#include <unordered_set>

/* $Id$ */
//  transactor <ref db> <test DBs>
//  expects berkley DBs of hash->file
int main(int argc, char *argv[])
{
        DBT key, value; 
        DB *db;
	DB *refdb[6];  // XXX
	int nrefdb = 0;
	int i, j;
        char *ks, *vs;
	std::unordered_set<std::string> hashes;
	std::vector<char *> misses;

        db = dbopen(argv[1], O_RDONLY, 0, DB_BTREE, NULL);
	
	for (i = 2; i < argc; i++) {
		refdb[i-2] = dbopen(argv[i], O_RDONLY, 0, DB_BTREE, NULL);
		if (refdb[i-2] == nullptr) {
			fprintf(stderr, "dbopen: %s\n", argv[i]);
			fflush(stderr);
		}
		assert(refdb[i-2] != nullptr);
		nrefdb++;
	}
	for (j = 0; j < nrefdb; j++) {
		for (;;) {
			i = refdb[j]->seq(refdb[j], &key, &value, R_NEXT);
			if (i != 0)
				break;
               		vs = (char*)calloc(1, value.size+1);
                	bcopy(value.data, vs, value.size);
			//printf("Insert %s\n", vs);
			hashes.insert(std::string(vs));
			free(vs);
		}
		refdb[j]->close(refdb[j]);
	}

	int nRecords = 0;
        for (;;) {
                i = db->seq(db, &key, &value, R_NEXT);
                if (i != 0)
                        break;
                ks = (char*)calloc(1, key.size+1);
                vs = (char*)calloc(1, value.size+1);
                bcopy(key.data, ks, key.size);
                bcopy(value.data, vs, value.size);
		//printf("Trying %s\n", vs);
		if (hashes.count(vs) == 0) {
			misses.push_back(ks);
		} else {
                	free(ks);
		}
		nRecords++;
                free(vs);
        }

        db->close(db);
	for (i = 0; i < misses.size(); i++) {
		printf("%s\n", misses[i]);
	}
	printf("misses: %d\n", misses.size());
	printf("nRecords: %d\n", nRecords);
}

