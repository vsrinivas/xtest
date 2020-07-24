#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/wait.h>

main(argc, argv)
        int argc;
        char *argv[];
{
        char cbuf[1000];
        int i;
        int j;
        int rc;

        j = 0;
        for (i = 1; i < argc; i++) {
                j += sprintf(&cbuf[j], "%s ", argv[i]);
        }

        for (;;) {
                rc = system(cbuf);
        }
}
