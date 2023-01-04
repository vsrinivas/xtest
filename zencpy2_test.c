#include <assert.h>

int main(int argc, char *argv[]) {
	char buf[10240];
	char buf2[10240];

	for (int i = 0; i < 10240; i++) {
		memset(buf, 0xaa, 10240);
		memset(buf2, 0, 10240);

		_zencpy2(buf2, buf, i);

		for (int j = 0; j < i; j++) {
			assert(buf[j] == buf2[j]);
		}
	}
}
