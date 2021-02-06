int main(int argc, char *argv[]) {

	// 2**10 = 1024. i want the last two digits, lets see!
	// mask based on 'digits';
	//	-- if its > 100?
	//	
	int i;
	unsigned long acc = 1;
	for (i = 0; i < 10; i++) {
		acc = 2 * acc;
		acc %= 100;	/// keep the 1's place.
	}
	printf("%ld\n", acc);


	// 2^7830457
	acc = 1;
	for (i = 0; i < 7830457; i++) {
		acc = 2 * acc;
		acc %= (10000000000ul);
	}
	printf("%lu\n", acc);

	acc *= 28433;
	acc += 1;

	printf("%lu\n", acc);
}
