#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

// wait until loadavg < 1.5 . if opt arg is passed in, max minutes to wait.
int main(int argc, char *argv[]) {
	char buf[180];
	char *ind;
	FILE *f;
	char *x, *y, *z, *q;
	double d;
	int minutes = 0;
	int maxminutes;

	if (argc > 1)
		maxminutes = atoi(argv[1]);
	else
		maxminutes = INT_MAX;

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
		d = atof(x);
#ifdef DEBUG
		printf("X %s Y %s Z %s d=%f\n", x, y, z, d);
#endif
		if (d < 1.5)
			break;
		if (access("/tmp/xwait_greenlight", R_OK) == 0)
			break;

		sleep(60);
		minutes++;
		if (minutes >= maxminutes)
			break;
	}
}
