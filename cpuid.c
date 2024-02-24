#include <cpuid.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
        unsigned long a, b, c, d;
	unsigned char mfg[12 + 1];
	unsigned long f, m, s;

	__cpuid_count(0x0, 0x0, a, b, c, d);
        printf("CPUID.(EAX=0h,ECX=0) %lx %lx %lx %lx\n", a, b, c, d);
	memcpy(&mfg[0], &b, 4);
	memcpy(&mfg[4], &d, 4);
	memcpy(&mfg[8], &c, 4);
	mfg[12] = 0;
	printf("%s\n", mfg);

	/* Brand String */
	unsigned int brand[12];
	__get_cpuid(0x80000002, brand+0x0, brand+0x1, brand+0x2, brand+0x3);
	__get_cpuid(0x80000003, brand+0x4, brand+0x5, brand+0x6, brand+0x7);
	__get_cpuid(0x80000004, brand+0x8, brand+0x9, brand+0xa, brand+0xb);
	printf("%s\n", brand);

	__cpuid_count(0x1, 0x0, a, b, c, d);
        printf("CPUID.(EAX=01h,ECX=0) %lx %lx %lx %lx\n", a, b, c, d);
	f = ((a >> 8) & 0xf) | ((a >> 16) & 0xff0);
	m = ((a >> 4) & 0xf) | ((a >> 12) & 0xf0);
	s = (a & 0xf);
	printf("Family %x Model %x Stepping %x\n", f, m, s);
	printf("cpu_type=%x\n", a & 0xF0FF0);
	if (c & (1 << 9))
		printf("ssse3 ");
	if (c & (1 << 19))
		printf("sse4.1 ");
	if (c & (1 << 26))
		printf("xsave ");
	if (c & (1 << 27))
		printf("osxsave ");
	if (c & (1 << 28))
		printf("avx ");
	if (c & (1 << 31))
		printf("hypervisor ");
	printf("\n");

	/* CPUID.(EAX=07h,ECX=0) */
	__cpuid_count(0x7, 0x0, a, b, c, d);
        printf("CPUID.(EAX=07h,ECX=0) %lx %lx %lx %lx\n", a, b, c, d);
	if (b & (1 << 3))
		printf("bmi1 ");
	if (b & (1 << 5))
		printf("avx2 ");
	if (b & (1 << 7))
		printf("smep ");
	if (b & (1 << 9))
		printf("erms ");
	if (b & (1 << 16))
		printf("avx512f ");
	if (b & (1 << 17))
		printf("avx512-dq ");
	if (b & (1 << 20))
		printf("smap ");
	if (b & (1 << 21))
		printf("avx512-ifma ");
	if (c & (1 << 5))
		printf("waitpkg ");
	if (c & (1 <<11))
		printf("avx512-vnni ");
	if (d & (1 << 4))
		printf("fsrm ");
	if (d & (1 << 9))
		printf("ia32_mcu_opt_ctrl ");
	if (d & (1 << 10))
		printf("md_clear ");
	if (d & (1 << 22))
		printf("amx-bf16 ");
	if (d & (1 << 24))
		printf("amx-tile ");
	if (d & (1 << 25))
		printf("amx-int8 ");
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

	/* CPUID.(EAX=07h,ECX=1) */
	__cpuid_count(0x7, 0x1, a, b, c, d);
        printf("CPUID.(EAX=07h,ECX=1) %lx %lx %lx %lx\n", a, b, c, d);
	if (a & (1 << 10))
		printf("fast_zero_rep_movsb ");
	if (a & (1 << 11))
		printf("fast_short_rep_stosb ");
	if (a & (1 << 12))
		printf("fast_short_rep_cmpsb_scasb ");
	if (a & (1 << 21))
		printf("amx-fp16 ");
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

	/* Fn8000_0001 Extended Processor Info and Feature Bits */
        __cpuid_count(0x80000001, 0, a, b, c, d);
        printf("CPUID.(EAX=8000_0001h,ECX=0) %lx %lx %lx %lx\n", a, b, c, d);
	if (c & (1 << 17))
		printf("tce ");
	if (c & (1 << 29))
		printf("monitorx ");
        printf("\n");

	/* Fn8000_0008_EBX Extended Feature Identifiers */
        __cpuid_count(0x80000008, 0, a, b, c, d);
        printf("CPUID.(EAX=8000_0008h,ECX=0) %lx %lx %lx %lx\n", a, b, c, d);
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
	if (b & (1 << 30))
		printf("IBPB_RET ");
        printf("\n");

	/* Fn8000_0021_EAX Extended Feature Identification 2 */
        __cpuid_count(0x80000021, 0, a, b, c, d);
        printf("CPUID.(EAX=8000_0021h,ECX=0) %lx %lx %lx %lx\n", a, b, c, d);
	if (a & (1 << 2))
		printf("LFenceAlwaysSerializing ");
	if (a & (1 << 7))
		printf("UpperAddressIgnore ");
	if (a & (1 << 8))
		printf("AutomaticIBRS ");
	if (a & (1 << 27))
		printf("SBPB ");
	if (a & (1 << 28))
		printf("IBPB_BRTYPE ");
	if (a & (1 << 29))
		printf("SRSO_NO ");
	if (a & (1 << 30))
		printf("SRSO_USER_KERNEL_NO ");
	if (a & (1 << 31))
		printf("SRSO_MSR_FIX ");
	printf("\n");
}
