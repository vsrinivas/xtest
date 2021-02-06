// find the unique positive integer whose square has the form:
//	1_2_3_4_5_6_7_8_9_0



/// 3 Jan 2021

//
//	1. min is:
//		sqrt(1020304050607080900) = 1010101010
//	2. max is:
//		sqrt(1929394959697989990) = 1389026623
//	3. last digit must be 0
//		


int is_good(unsigned long n) {
	char buf[64] = {};
	int i;

	sprintf(buf, "%lu", n);
	i = strlen(buf);
	i--;

	if (buf[i] == '0' &&
	    buf[i - 2] == '9' &&
	    buf[i - 4] == '8' &&
	    buf[i - 6] == '7' &&
	    buf[i - 8] == '6' &&
	    buf[i - 10] == '5' &&
	    buf[i - 12] == '4' &&
	    buf[i - 14] == '3' &&
	    buf[i - 16] == '2' &&
	    buf[i - 18] == '1')
		return 1;
	return 0;
}


int main(int argc, char *argv[]) {
	unsigned long xmin = 1010101010ul;
	unsigned long xmax = 1389026623ul;

	for (unsigned long n = xmin; n < xmax; n += 10) {
		if (is_good(n * n)) {
			printf("%lu\n", n);
			return 0;
		}
	}
}
