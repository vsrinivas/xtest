// python -m http.server 8000 --bind 127.0.0.1

#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

main(argc, argv)
	char *argv[];
{
	char *metricsdir;
	char buf[80];
	int metricsfile;
	int rc;
	int port;

	port = (argc == 1) ? 8000 : atoi(argv[1]);

	sprintf(buf, "/tmp/%d", getpid());
	mkdir(buf, 0700);

	sprintf(buf, "/tmp/%d/XXXXXX", getpid());
	mkdtemp(buf);
	metricsdir = strdup(buf);

	bzero(buf, sizeof(buf));
	sprintf(buf, "%s/metrics", metricsdir);
	metricsfile = open(buf, O_CREAT | O_RDWR, 0700);
	if (metricsfile == -1)
		return -1;

	rc = fork();
	if (rc == 0) {
		chdir(metricsdir);
		bzero(buf, sizeof(buf));
		sprintf(buf, "python3 -m http.server %d --bind 127.0.0.1\n", port);
		system(buf);
		return 0;
	}
	wait(NULL);
	return 0;
}
