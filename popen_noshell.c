#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <spawn.h>
#include <errno.h>

FILE *xpopen(const char *command[], const char *type) {
	pid_t pid;
	int rc;
	posix_spawn_file_actions_t actions;
	posix_spawnattr_t attr;
	int pfd[2];
	int i;
	int src, tgt, mine;
	char *env[] = { NULL };

	pipe(&pfd);	/* XXX rc */
	if (type[0] == 'r') {
		src = 1, tgt = 1, mine = 0;
	} else {
		src = 0, tgt = 0, mine = 1;
	}

	posix_spawnattr_init(&attr);

	posix_spawn_file_actions_init(&actions);
	posix_spawn_file_actions_adddup2(&actions, pfd[src], tgt);

	rc = posix_spawn(&pid, command[0], &actions, &attr, command, env);
	if (rc != 0) {
		printf("%d %d\n", errno, rc);
	}

	close(pfd[src]);
	posix_spawn_file_actions_destroy(&actions);
	return fdopen(pfd[mine], type);	
}

int xpclose(FILE *stream) {
	fclose(stream);
	return 0;
}

#ifdef TEST
int main() {
	const char *cmd1[] = { "/bin/ls", "-l", "File Name With Spaces", NULL };
	FILE *f;
	char buf[180];

	f = xpopen(cmd1, "r");
	while (!feof(f)) {
		bzero(buf, 180);
		fgets(buf, 180, f);
		printf("> %s\n", buf);
	}
	xpclose(f);
}
#endif
