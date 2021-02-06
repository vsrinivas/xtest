#include <vector>
#include <unordered_set>
#include <algorithm>
#include <stdio.h>
#include <stdint.h>

using ball = uint8_t;

int main(int argc, char *argv[]) {
	int trials = atoi(argv[1]);
	std::vector<ball> urn;
	unsigned long sum = 0;
	unsigned long sums[128] ={};

	int color = -1;
	for (int j = 0; j < 70; j++) {
		if ((j % 10) == 0) color++;
		urn.push_back(color);
	}
	for (int i = 0; i < trials; i++) {
		std::vector<ball> l_urn = urn;
		std::random_shuffle(l_urn.begin(), l_urn.end());

		std::unordered_set<ball> hand;
		for (int j = 0; j < 20; j++) {
			hand.insert(l_urn.back());
			l_urn.pop_back();
		}
		int sz = hand.size();
		sums[0] += sz;
	}

	for (int i = 0 ; i <  128; i++) sum += sums[i];
	double avg = (1.0 * sum) / trials;
	printf("%f\n", avg);
}
