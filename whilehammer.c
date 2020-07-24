#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/wait.h>

main(argc, argv)
        int argc;
        char *argv[];
{
        int N = atoi(argv[1]);
        char *cbuf[1000];
        int i;
        int j;
        int rc;

        j = 0;
        for (i = 2; i < argc; i++) {
                j += sprintf(&cbuf[j], "%s ", argv[i]);
        }

        for (;;) {
                for (i = 0; i < N; i++) {
                        j  = fork();
                        if (j == 0) {
                                rc = system(cbuf);
                                assert(WEXITSTATUS(rc) == 0);
                                return rc;
                        }
                }

                for (i = 0; i < N; i++) {
                        wait(&rc);
                        assert(WEXITSTATUS(rc) == 0);
                }
        }
}
