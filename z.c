#define _GNU_SOURCE 
#include <sys/syscall.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int memfd_create(const char* name, unsigned int flags)
{
	#if __aarch64__
	#define __NR_memfd_create 279
	#elif __arm__
	#define __NR_memfd_create 279
	#elif __powerpc64__
	#define __NR_memfd_create 360
	#elif __i386__
	#define __NR_memfd_create 356
	#elif __x86_64__
	#define __NR_memfd_create 319
	#endif /* __NR_memfd_create__ */
	#ifdef __NR_memfd_create
		// In the event of no system call this returns -1 with errno set
		// as ENOSYS.
		return syscall(__NR_memfd_create, name, flags);
	#else
		return -1;
	#endif
}

#include <errno.h>

main() {
	int fd = memfd_create("hiiii", 0);
	ftruncate(fd, 4096);
	char buf[80];
	sprintf(buf, "/proc/%d/fd/%d", getpid(), fd);

	char c[80];
	sprintf(c, "ls  -l /proc/%d/fd\n", getpid());
	system(c);
	printf("\n\n");

	struct stat sb;
	int rc = stat(buf, &sb);
	printf("stat rc=%d errno=%d\n", rc, errno);
	int fd2 = open(buf, O_RDONLY);
	printf("fd2=%d errno=%d\n", fd2, errno);
	fchmod(fd2, 00400);

	// write via fd1;
	char oldmac[] = "old macdonald had a farm";
	write(fd, oldmac, strlen(oldmac));

	// read via fd2. expect old mac.
	char farmbuf[70];
	read(fd2, farmbuf, 70);
	printf("expected %s got %s\n", oldmac, farmbuf);

	// write via fd2. shouldn't work.
	int rc2 = write(fd2, oldmac, strlen(oldmac));
	printf("write fd2 rc %d errno %d\n", rc2, errno);


	// try to change perms on fd2:
	int rc3 = fcntl(fd2, F_SETFL, O_RDWR);
	printf("fnctl %d\n", rc3);
	int rc4 = write(fd2, oldmac, strlen(oldmac));
	printf("write fd2 rc %d errno %d\n", rc4, errno);


	// can we make it wr?
	char zuf[55];
	sprintf(zuf, "/proc/self/fd/%d", fd2);
	int fd3 = open(zuf, O_RDWR);
	printf("fd3=%d\n", fd3);
}
