#include <stdio.h>


double load(void)
{
	char buf[180];
	FILE *f;
	char *ind;
	char *x, *y, *z, *q;
	double d;

	bzero(buf, sizeof(buf));
	f = popen("uptime", "r");
	fgets(buf, sizeof(buf), f);
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
	//      printf("X %s Y %s Z %s\n", x, y, z);
	d = atof(x);
	return d;
}

int main(int argc, char *argv[]) {
	int numproc;
	char *p;
	int i;
	FILE* cmd;
	int cmds;
	char buf[280];
	char **cmdvec;
	FILE **procs;

	numproc = 4;
	p = getenv("NUMPROC");
	if (p)
		numproc = atoi(p);

	cmds = 0;
	cmd = fopen(argv[1], "r");
	while (!feof(cmd)) {
		bzero(buf, sizeof(buf));
		fgets(buf, sizeof(buf), cmd);
		if (strlen(buf) == 0)
			break;
		cmds++;
	}
	rewind(cmd);

	i = 0;
	cmdvec = calloc(cmds, sizeof(char*));
	while (!feof(cmd)) {
		bzero(buf, sizeof(buf));
		fgets(buf, sizeof(buf), cmd);
		if (strlen(buf) == 0)
			break;
		if (buf[0] == '#') {
			i++;
			continue;
		}
		cmdvec[i++] = strdup(buf);
	}

	double l;
	int j;
	procs = calloc(cmds, sizeof(FILE*));
	for (i = 0; i < cmds; i++) {
		if (cmdvec[i])
			procs[i] = popen(cmdvec[i], "r");

		for (j = 0; j < 10; j++) {
			l = load();
			if (l > numproc) {
				sleep(5);
			} else {
				break;
			}
		}
	}
	for (i = 0; i < cmds; i++) {
		if (!procs[i])
			continue;
		while (!feof(procs[i])) {
			bzero(buf, sizeof(buf));
			fgets(buf, sizeof(buf), procs[i]);
			if (strlen(buf) == 0)
				break;
			printf("%d %s", i, buf); 
		}
	}
	for (i = 0; i < cmds; i++) {
		if (!procs[i])
			continue;
		pclose(procs[i]);
	}
}
