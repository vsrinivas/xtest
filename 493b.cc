#include <array>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <stdint.h>

using ball = uint8_t;

int main(int argc, char *argv[]) {
	int trials = atol(argv[1]);
	std::vector<ball> urn(70);
	unsigned long sum = 0;

	int color = -1;
	for (int j = 0; j < 70; j++) {
		if ((j % 10) == 0) color++;
		urn[j] = color;
	}
	for (unsigned long i = 0; i < trials; i++) {
		std::vector<ball> l_urn = urn;
		std::array<uint8_t, 7> hits = {};
		std::random_shuffle(l_urn.begin(), l_urn.end());

		for (int j = 0; j < 20; j++) {
			hits[l_urn.back()] = 1;
			l_urn.pop_back();
		}
		if (hits[0] == 0)
			sum++;

	}
	double avg = (1.0 * sum) / trials;
	printf("%f\n", avg);
}
