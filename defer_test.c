#include "defer.h"
#include <stdio.h>


int main(int argc, char *argv[]) {
	defer_init();

	defer_sync();
}
