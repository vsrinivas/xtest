#include <stdio.h>
#include <math.h>
#include <stdlib.h>

unsigned long get_n(unsigned long b) {
    if (b == 0) return 1;
    unsigned long n = 1;
    while (n <= b) {
        n *= 10;
    }
    return n;
}

int main(int argc, char *argv[]) {
    unsigned long t = 4;
    if (argc > 1) t = strtoul(argv[1], NULL, 10);

    unsigned long hlimit = 1;
    for (unsigned long i = 0; i < t/2; i++) hlimit *= 10;
    hlimit -= 1;

    unsigned long s = 0;

    // The original code required b to be a perfect square
    // We start rootB = 1 to avoid log10(0) issues and match original behavior
    for (unsigned long rootB = 1; rootB * rootB <= hlimit; rootB++) {
        unsigned long b = rootB * rootB;
        unsigned long n = get_n(b);

        unsigned __int128 D = (unsigned __int128)n * n - (unsigned __int128)4 * b * n + (unsigned __int128)4 * b;

        unsigned long rootD = (unsigned long)sqrt((double)D);
        if ((unsigned __int128)rootD * rootD == D) {
            // Root 1: a = (n - 2*b + rootD) / 2
            unsigned __int128 num1 = (unsigned __int128)n - (unsigned __int128)2 * b + rootD;
            if (num1 % 2 == 0) {
                unsigned long a1 = (unsigned long)(num1 / 2);
                if (a1 > 0 && a1 <= n) {
                    unsigned long u1 = n * a1 + b;
                    unsigned long sum_ab1 = a1 + b;
                    if (u1 == sum_ab1 * sum_ab1) {
                        printf("%lu\n", u1);
                        s += u1;
                    }
                }
            }

            // Root 2: a = (n - 2*b - rootD) / 2
            if ((unsigned __int128)n > (unsigned __int128)2 * b + rootD) {
                unsigned __int128 num2 = (unsigned __int128)n - (unsigned __int128)2 * b - rootD;
                if (num2 % 2 == 0) {
                    unsigned long a2 = (unsigned long)(num2 / 2);
                    if (a2 > 0 && a2 <= n) {
                        unsigned long u2 = n * a2 + b;
                        unsigned long sum_ab2 = a2 + b;
                        if (u2 == sum_ab2 * sum_ab2) {
                            printf("%lu\n", u2);
                            s += u2;
                        }
                    }
                }
            }
        }
    }
    printf("  %lu\n", s);
    return 0;
}
