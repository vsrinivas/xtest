#include <stdint.h>

static unsigned int bit(uint32_t value, int i) { return !!(value & (1 << i)); }
static uint32_t lfsr24(uint32_t value) {
	/* 24-bit maximal length LFSR; taps 24, 23, 22, 17 */
	unsigned int feedback = bit(value, 23) ^
				bit(value, 22) ^
				bit(value, 21) ^
				bit(value, 16);
	return ((value << 1) | feedback) & ((1 << 24) - 1);
}

