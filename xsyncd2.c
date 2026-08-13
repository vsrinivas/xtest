#include <time.h>
#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>

void uptime(char *out, int n) {
	char buf[80] = {};
	FILE *f;
	char *ind;
	char *x, *y;

	bzero(out, n);
	f = popen("uptime", "r");
	fgets(buf, sizeof(buf), f);
	pclose(f);
	// load average: 9.73, 9.97, 9.51
	ind = rindex(buf, ':');
	ind++;
	ind++;
	x = ind;
	y = rindex(buf, '\n');
	*y = 0;
	strncpy(out, x, n);
}

#define N 32
time_t tms[N];
int main(int argc, char *argv[])
{
	char *metricsdir;
	char buf[80];
	int metricsfile;
	int rc;
	int port;

	int i = 0, n;
        time_t a, b;
	for (;;) {
	        a = time(NULL);
                sync();
                b = time(NULL);
                tms[i++] = b - a;

                if (i == N - 1) {
			 time_t now;
			 now = time(NULL);
			char *p = ctime(&now);
			char *q;

			char ubuf[80];
			uptime(ubuf, 80);

			q = index(p, '\n');
			*q = 0;
			printf("[%s] %s, ", p, ubuf);
                        for (n = 0; n < N-1; n++) {
                                printf("%d ", tms[n]);
                        }
                        printf("\n");
                        i = 0;
                }
		sleep(1);
	}


	wait(NULL);
	return 0;
}
