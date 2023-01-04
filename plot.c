#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

// write out loadavg to a file;
main(int argc, char *argv[]) {
	char buf[180];
	char *ind;
	FILE *f;
	char *x, *y, *z, *q;
	double d;
	FILE *log;

	chdir("/home/vsrinivas/bin");
	log = fopen("plot.log", "a");

	for (;;) {
		bzero(buf, 180);
		f = popen("uptime", "r");
		fgets(buf, 180, f);
		pclose(f);
		ind = rindex(buf, ':');
		// load average: 9.73, 9.97, 9.51
		ind++;
		ind++;
		x = ind;
		y = index(x, ',');
		*y = 0;
		y++;
		y++;
		z = index(y, ',');
		*z = 0;
		z++;
		z++;
		q = index(z, '\n');
		*q = 0;
		d = atof(x);	// 1s avg

		bzero(buf, 180);
		f = popen("date +%s", "r");
		fgets(buf, 180, f);
		pclose(f);
		x = index(buf, '\n');
		*x = 0;

		fprintf(log, "%s, %f\n", buf, d);
		fflush(log);
		sleep(2);
	}
}
