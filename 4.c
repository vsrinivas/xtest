int ispalindrome(int x) {
	char buf[64] = {};
	int digs;
	int i;
	sprintf(buf, "%d", x);
	digs = strlen(buf);
	for (i = 0; i < digs/2; i++) {
		if (buf[i] != buf[digs - i-1]) {
			return 0;
		}
	}
	return 1;
}

int main(int argc, char *argv[]) {
	int u, v;
	int uv;
	int maxuv = 0;

	for (u = 999; u >= 100; u--) {
		for (v = 999; v >= 100; v--) {
			uv = u * v;
			if (ispalindrome(uv)) {
				if (uv > maxuv) {
					maxuv = uv;
					printf("%d\n", maxuv);
				}
			}
		}
	}

}
