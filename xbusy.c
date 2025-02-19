#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

// true if loadavg > 1.5
main(int argc, char *argv[]) {
	char buf[180];
	char *ind;
	FILE *f;
	char *x, *y, *z, *q;
	double d;

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
	printf("X %s Y %s Z %s d=%f\n", x, y, z, d);
	if (d < 1.5)
		return 0;
	return 1;
}
