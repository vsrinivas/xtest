int fib(int x) {
        if (x == 0) return 0;
        if (x == 1) return 1;
        if (x == 2) return 1;
        return fib(x-1) + fib(x-2);
}
extern unsigned long now(void);
main(int argc, char *argv[]) {
        //hammertime();
        unsigned long a, b;
        int x = atoi(argv[1]);
        a = now();
        fib(x);
        b=now();
        printf("%d %ld\n", x, b-a);
}
