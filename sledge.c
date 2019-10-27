#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

//#define MAX_BLOCKS		(5859375000ull)		/* 3TB / 512 */
#define MAX_BLOCKS		(10000000)
#define TRAJ			(100000ull)

static bool genbool(int p_bad) {
	int raw = rand() % 1000;
	return (raw < p_bad);
}

int main(int argc, char *argv[]) {
	int p_bad;
	int to_sample;
	int traj;
	bool match;

	uint64_t blocks_to_sample;
	uint64_t found, missed;

	uint64_t i, j;

	// % of blocks bad
	for (p_bad = 1; p_bad < 100; p_bad++) {
		// % to sample
		for (to_sample = 1; to_sample < 100; to_sample++) {
			blocks_to_sample = (MAX_BLOCKS * to_sample) / 1000000;
			found = 0;
			missed = 0;

			for (traj = 0; traj < TRAJ; traj++) {
				match = false;
				for (i = 0; i < blocks_to_sample; i++) {
					match |= genbool(p_bad);
					if (match)
						break;
				}

				if (match)
					found++;
				else
					missed++;
			}

			// For |p_bad| percent blocks bad, |to_sample| % sampled,
			// found/{found + missed} percent of the time we found
			// the thing.
			printf("p_bad=%d (%lu), to_sample=%d (%lu %f percent), pct=%f\n",
				p_bad, (p_bad * MAX_BLOCKS) / 1000,
				to_sample, blocks_to_sample, (1.0 * blocks_to_sample) / MAX_BLOCKS * 100,
				(1.0 * found) / (found + missed) * 100);
		}
	}
}
