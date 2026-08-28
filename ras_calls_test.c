#include <stdio.h>
#include <assert.h>
#include <stdint.h>

static uint64_t depth;

void ras_calls(uint64_t *, int);
void ras_calls2(uint64_t *, int, void * /*ret_addr*/);

int main(int argc, char *argv[]) {
	int calls = 50000;

	/* Deep recursive CALL/RET, increments counter on the way out */
	ras_calls(&depth, calls);
	assert(depth == calls);
	
	/* Deep recursive CALL, but one level of JMP to return */
	depth = 0;
	ras_calls2(&depth, calls, NULL);
	assert(depth == calls);
}
