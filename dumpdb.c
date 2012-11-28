#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <fcntl.h>
#include <db.h>

main(argc, argv)
	int argc;
	char *argv[];
{
	DBT key, value;	
	DB *db;
	int i;
	char *ks, *vs;

	db = dbopen(argv[1], O_RDONLY, 0, DB_BTREE, NULL);

	for (;;) {
		i = db->seq(db, &key, &value, R_NEXT);
		if (i != 0)
			break;
		ks = calloc(1, key.size+1);
		vs = calloc(1, value.size+1);
		bcopy(key.data, ks, key.size);
		bcopy(value.data, vs, value.size);
		printf("%s %s\n", ks, vs);
		free(ks);
		free(vs);
	}

	db->close(db);
}
