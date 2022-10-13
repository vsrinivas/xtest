#include <cpuid.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
        unsigned long a, b, c, d;

	/* CPUID.(EAX=07h,ECX=0) */
	__cpuid_count(0x7, 0x0, a, b, c, d);
        printf("CPUID.(EAX=07h,ECX=0) %lx %lx %lx %lx\n", a, b, c, d);
	if (d & (1 << 9))
		printf("ia32_mcu_opt_ctrl ");
	if (d & (1 << 10))
		printf("md_clear ");
	if (d & (1 << 26))
		printf("ibrs ");
	if (d & (1 << 27))
		printf("stibp ");
	if (d & (1 << 28))
		printf("l1d_flush ");
	if (d & (1 << 29))
		printf("ia32_arch_capabilities ");
	if (d & (1 << 30))
		printf("ia32_core_capabilities ");
	if (d & (1 << 31))
		printf("ssbd ");
        printf("\n");

	/* CPUID.(EAX=07h,ECX=2) */
	__cpuid_count(0x7, 0x2, a, b, c, d);
        printf("CPUID.(EAX=07h,ECX=2) %lx %lx %lx %lx\n", a, b, c, d);
	if (d & (1 << 0))
		printf("psfd ");
	if (d & (1 << 1))
		printf("ipred_ctrl ");
	if (d & (1 << 2))
		printf("rrsba_ctrl ");
	if (d & (1 << 4))
		printf("bhi_ctrl ");
	if (d & (1 << 5))
		printf("mcdt_no ");
        printf("\n");

	/* Fn8000_0008_EBX Extended Feature Identifiers */
        __cpuid_count(0x80000008, 0, a, b, c, d);
        printf("Fn8000_0008 %lx %lx %lx %lx\n", a, b, c, d);

        if (b & (1 << 12))
                printf("ibpb ");
        if (b & (1 << 14))
                printf("ibrs ");
        if (b & (1 << 15))
                printf("stibp ");
        if (b & (1 << 16))
                printf("ibrs_always_on ");
        if (b & (1 << 17))
                printf("stibp_always_on ");
        if (b & (1 << 18))
                printf("ibrs_preferred ");
        if (b & (1 << 19))
                printf("IbrsSameMode ");
        if (b & (1 << 20))
                printf("EferLmsleUnsupported ");
        if (b & (1 << 24))
                printf("ssbd ");
        if (b & (1 << 25))
                printf("virt_spec_ctrl ");
        if (b & (1 << 26))
                printf("SsbdNotRequired ");
        if (b & (1 << 28))
                printf("psfd ");
        if (b & (1 << 29))
                printf("BTC_NO ");
        printf("\n");
}
