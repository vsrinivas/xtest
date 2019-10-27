// g++ ftwdb2log.cc  -std=c++11 -lsasl2 -o ftw2db2log 
#define _XOPEN_SOURCE 700
#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <ftw.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <sasl/md5global.h>
#include <sasl/md5.h>

#define MD5_DIGEST_LENGTH	(16)

static char *md5sumbuflocal(const char *buf, size_t size) {
	char sum[2 * MD5_DIGEST_LENGTH + 1] = {};
	unsigned char hash_buf[MD5_DIGEST_LENGTH] = {};
	int i, j;

	MD5_CTX hash_context;
	_sasl_MD5Init(&hash_context);
	_sasl_MD5Update(&hash_context, (unsigned char *) buf, size);
	_sasl_MD5Final(hash_buf, &hash_context);

	for (i = 0, j = 0; i < MD5_DIGEST_LENGTH; i++)
		j += sprintf(&sum[j], "%02x", hash_buf[i]);
	return strdup(sum);
}

/* MD5 a file */
// Thread-hostile.
static char *md5sumg(int fd, const struct stat *sb) {
	static char bounce[262144];
	char *p; 
	char *q;
	bool do_mmap;
	int rc;

	do_mmap = !(sb->st_size <= sizeof(bounce));

	if (do_mmap) {
		p = (char*) mmap(NULL, sb->st_size, PROT_READ, MAP_SHARED, fd, 0);
		if (p == MAP_FAILED) {
			close(fd);
			return NULL;
		}
	} else {
		rc = pread(fd, bounce, sb->st_size, 0);
		if (rc != sb->st_size) {
			close(fd);
			return NULL;
		}
		p = (char *) bounce;
	}

	q = md5sumbuflocal((const char *) p, sb->st_size);
	if (do_mmap) {
		munmap(p, sb->st_size);
	}
	close(fd);
	return q;
}

static int db;
static int nFiles;

static int db_write(char *value) {
	int rc;
	char buf[32 + 1 + 1];

	memset(buf, 0, 32 + 1 + 1);
	sprintf(buf, "%s\n", value);
	rc = write(db, buf, 32 + 1);
	return rc;
}

int cb(const char *path, const struct stat *sb, int typeflag, struct FTW *) {
	char* val;
	int rc = 0;

	switch (typeflag) {
	case FTW_F: {
		if ((sb->st_mode & S_IFMT) != S_IFREG)
			break;

		if (sb->st_size == 0)
			break;

		int fd = open(path, O_RDONLY);
		if (fd == -1) {
			printf("open error: %s\n", path);
			rc = -1;
			break;
		}
		val = md5sumg(fd, sb);
		if (!val) {
 			printf("md5sum error: %s\n", path);
			rc = -1;
			break;
		}
		printf("==> %s, %s\n", path, val);

		++nFiles;
		auto s = db_write(val);
		if (s == -1) {
 			printf("leveldb error: %s\n", path);
			rc = -1;
		}
		free(val);
		break;
	}
	default:
		break;
	}

	return rc;
}

/* ftwdb2_leveldb <path> <dbfile> */
/* Make a log mapping every file to its MD5SUM (in 32-byte ascii string) */
int main(int argc, char *argv[])
{
	int i;

	db = open(argv[2], O_RDWR | O_CREAT, 0700);
	if (db == -1) {
		printf("Error creating db\n");
		return -1;
	}

	i = nftw(argv[1], cb, 64, FTW_PHYS);
	if (i) printf("%d err %d\n", i, errno);

	fsync(db);
	close(db);
	printf("%d files\n", nFiles);
}
