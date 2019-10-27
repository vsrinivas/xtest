#define	roundup(x, y)	((((x)+((y)-1))/(y))*(y))

main() {
	int i = 4096;
	printf("%d\n", roundup(i, 4096));
}
