#define _XOPEN_SOURCE 500
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

#include <leveldb/db.h>
#include <openssl/md5.h>

#define	roundup(x, y)	((((x)+((y)-1))/(y))*(y))

static char *md5sumbuflocal(const char *buf, size_t size) {
	char sum[2 * MD5_DIGEST_LENGTH + 1] = {};
	unsigned char hash_buf[MD5_DIGEST_LENGTH] = {};
	int i, j;

	MD5_CTX hash_context;
	MD5_Init(&hash_context);
	MD5_Update(&hash_context, buf, size);
	MD5_Final(hash_buf, &hash_context);

	for (i = 0, j = 0; i < MD5_DIGEST_LENGTH; i++)
		j += sprintf(&sum[j], "%02x", hash_buf[i]);
	return strdup(sum);
}

/* MD5 a file */
// Thread-hostile.
static char *md5sumg(int fd, const struct stat *sb) {
	static char bounce[262144];
	static char* last_mmap_ptr = NULL;		// XXX
	static char* mmap_low_watermark = NULL;
	char *p; 
	char *q;
	char *np; 
	bool do_mmap;
	int rc;

	do_mmap = !(sb->st_size <= sizeof(bounce));

	if (do_mmap) {
		p = (char*) mmap(last_mmap_ptr, sb->st_size, PROT_READ, MAP_FILE|MAP_SHARED|MAP_POPULATE, fd, 0);
		if (p == MAP_FAILED) {
			close(fd);
			return NULL;
		}
		np = p + roundup(sb->st_size, 4096);
		if (p != last_mmap_ptr)
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


static leveldb::DB* db;
static int nFiles;

int cb(const char *path, const struct stat *sb, int typeflag, struct FTW *) {
	char* val;
	int rc = 0;

	switch (typeflag) {
	case FTW_F: {
		if ((sb->st_mode & S_IFMT) != S_IFREG)
			break;

		if (sb->st_size == 0)
			break;

		leveldb::Slice key(path, strlen(path) + 1);

		int fd = open(path, O_RDONLY | O_NOATIME);
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
		//printf("==> %s, %s\n", path, val);

		leveldb::Slice value(val, 32 + 1);

		++nFiles;
		auto s = db->Put(leveldb::WriteOptions(), key, value);
		if (!s.ok()) {
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
/* Make a LevelDB mapping every file to its MD5SUM (in 32-byte ascii string) */
int main(int argc, char *argv[])
{
	int i;

	leveldb::Options options;
	options.create_if_missing = true;
	leveldb::Status status = leveldb::DB::Open(options, argv[2], &db);
	if (!status.ok()) {
		printf("Error creating db\n");
		return -1;
	}

	i = nftw(argv[1], cb, 64, FTW_PHYS);
	if (i) printf("%d err %d\n", i, errno);

        delete db;
	printf("%d files\n", nFiles);
}
