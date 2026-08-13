#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#define N (65536)
#define NUM_TARGETS 1536
#define TARGET_ALIGN 128

extern void add_start();

void shuffle(void (**array)(long long *), size_t n) {
    if (n > 1) {
        for (size_t i = 0; i < n - 1; i++) {
            size_t j = i + rand() % (n - i);
            void (*t)(long long *) = array[j];
            array[j] = array[i];
            array[i] = t;
        }
    }
}

void (*fp[N])(long long *);

static void xcall(long long *ctr, int i) {
	fp[i](ctr);
	i++;
	if (i < N)
		xcall(ctr, i);
}

int main(int argc, char *argv[]) {
    void (*fp2[NUM_TARGETS])(long long *);
    int i;
    int j;
    long long ctr1 = 0, ctr2 = 0;
    
    srand(time(NULL));

    for (i = 0; i < NUM_TARGETS; i++) {
        fp2[i] = (void (*)(long long *))((char *)add_start + i * TARGET_ALIGN);
    }

    for (i = 0; i < N; i++) {
        fp[i] = fp2[i % NUM_TARGETS];
    }

    for (i = 0; i < N; i++) {
        fp[i](&ctr1);
    }

    for (j = 0; j < 100; j++) {
        ctr2 = 0;
        shuffle(fp, N);
	xcall(&ctr2, 0);
        assert(ctr1 == ctr2);
    }

    printf("Done! ctr1: %lld, ctr2: %lld\n", ctr1, ctr2);
    return 0;
}
