#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <fcntl.h>
#include <db.h>
#include <assert.h>
#include <string.h>
#include "cp.h"
#include "md5.h"

main(argc, argv)
	int argc;
	char *argv[];
{
	DBT key, value;	
	DB *db;
	int i;
	char *ks, *vs, *dst, *dp;
	char *md;
	int rc;

	db = dbopen(argv[1], O_RDONLY, 0, DB_BTREE, NULL);
	assert(db != NULL);

	for (;;) {
		i = db->seq(db, &key, &value, R_NEXT);
		if (i != 0)
			break;
		ks = calloc(1, key.size+1);
		vs = calloc(1, value.size+1);
		bcopy(key.data, ks, key.size);
		bcopy(value.data, vs, value.size);
		dst = calloc(1, key.size + strlen(dp) + 1);
		sprintf(dst, "%s/%s", dp, ks);
		rc = copy_file(ks, dst);
		printf("%s -> %s %d %s\n", ks, dst, rc, vs);
		md = md5sum(dst);
		assert(rc == 0);
		assert(strcmp(vs, md) == 0);
		free(md);
		free(ks);
		free(vs);
	}

	db->close(db);
}
