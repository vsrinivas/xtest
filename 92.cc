#include <hash_set>

unsigned long sum_digs_sq(unsigned long n) {
        unsigned long s = 0;
        while (n) {
                s += ((n % 10) * (n %10));
                n = n / 10;
        }
        return s;
}

int main(int argc, char *argv[]) {
	__gnu_cxx::hash_set<int> ones;
	int i;
	int sum = 0;

	for (i = 1; i < 10000000; i++) {
//		printf("%d\n", i);
		int t = i;
		for (;;) {
			if (ones.count(t))
				break;
			int v = sum_digs_sq(t);
			if (v == 89) {
				sum++;
				break;
			}
			if (v == 1) {
				ones.insert(t);
				break;
			}
			t = v;
		}
	}
	printf("%d %d\n", eight_nine.size(), sum);
}
