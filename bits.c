main(argc, argv)
	char *argv[];
{
	unsigned long l;
	int i;

	l = strtol(argv[1], 0, 0);
	for (i = (sizeof(l) * 8) - 1; i >= 0; i--) {
		//printf("%d: %u\n", i, !!(l & (1ul << i)));
		printf("%d", !!(l & (1ul << i)));
		if (i && ((i % 8) == 0))
			printf("_");
	}
	printf("\n");
	printf("63");
	for (i = 0; i <31; i++)
	  printf(" ");
	printf("32");
	printf(" ");
	printf("31");
	for (i = 0; i <32; i++)
	  printf(" ");
	printf("0");
	printf("\n");
}

