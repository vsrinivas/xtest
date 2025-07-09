#include <stdio.h>

void x86_ras_fill(void);
void bhb_scrub_short(void);
void bhb_scrub_long(void);
unsigned long now(void);

int main(int argc, char *argv[]) {
	unsigned long a, b;
	int i;

	a = now();
	for (i = 0; i < 1000; i++)
		asm volatile("" ::: "memory");
	b = now();
	printf("No-op %lu %f\n", b-a, (1.0 * b-a)/1000);

	a = now();
	for (i = 0; i < 1000; i++)
		x86_ras_fill();
	b = now();
	printf("x86_ras_fill %lu %f\n", b-a, (1.0 * b-a)/1000);

	a = now();
	for (i = 0; i < 1000; i++)
		bhb_scrub_short();
	b = now();
	printf("bhb_scrub_short %lu %f\n", b-a, (1.0 * b-a)/1000);

	a = now();
	for (i = 0; i < 1000; i++)
		bhb_scrub_long();
	b = now();
	printf("bhb_scrub_long %lu %f\n", b-a, (1.0 * b-a)/1000);
}
