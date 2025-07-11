#include <cpuid.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
        unsigned long a, b, c, d;
	unsigned char mfg[12 + 1];
	unsigned long f, m, s;
	int amd = 0;

	__cpuid_count(0x0, 0x0, a, b, c, d);
        printf("CPUID.(EAX=0h,ECX=0) %lx %lx %lx %lx\n", a, b, c, d);
	memcpy(&mfg[0], &b, 4);
	memcpy(&mfg[4], &d, 4);
	memcpy(&mfg[8], &c, 4);
	mfg[12] = 0;
	printf("%s\n", mfg);
	if (strcmp(mfg, "AuthenticAMD") == 0)
		amd = 1;

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

	/* Monitor/MWAIT Features */
	__cpuid_count(0x5, 0x0, a, b, c, d);
        printf("CPUID.(EAX=05h,ECX=0) %lx %lx %lx %lx\n", a, b, c, d);

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
	if (a & (1 << 6))
		printf("lass ");
	if (a & (1 << 10))
		printf("fast_zero_rep_movsb ");
	if (a & (1 << 11))
		printf("fast_short_rep_stosb ");
	if (a & (1 << 12))
		printf("fast_short_rep_cmpsb_scasb ");
	if (a & (1 << 21))
		printf("amx-fp16 ");
	if (a & (1 << 22))
		printf("hreset ");
	if (a & (1 << 23))
		printf("avx-ifma ");
	if (a & (1 << 26))
		printf("lam ");
	if (a & (1 << 27))
		printf("msrlist ");
	if (a & (1 << 30))
		printf("invd_disable_post_bios_done ");
	if (d & (1 << 13))
		printf("utmr ");
	if (d & (1 << 14))
		printf("prefetchi ");
	if (d & (1 << 15))
		printf("user_msr ");
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
	if (d & (1 << 3))
		printf("ddpd_u ");
	if (d & (1 << 4))
		printf("bhi_ctrl ");
	if (d & (1 << 5))
		printf("mcdt_no ");
	if (d & (1 << 7))
		printf("monitor_mitg_no ");
        printf("\n");

	/* Fn8000_0001 Extended Processor Info and Feature Bits */
        __cpuid_count(0x80000001, 0, a, b, c, d);
        printf("CPUID.(EAX=8000_0001h,ECX=0) %lx %lx %lx %lx\n", a, b, c, d);
	if (c & (1 << 2))
		printf("SVM ");
	if (c & (1 << 5))
		printf("ABM ");
	if (c & (1 << 6))
		printf("SSE4A ");
	if (c & (1 << 7))
		printf("MisAlignSse ");
	if (c & (1 << 8))
		printf("3DNowPrefetch ");
	if (c & (1 << 10))
		printf("IBS ");
	if (c & (1 << 15))
		printf("lwp ");
	if (c & (1 << 16))
		printf("fma4 ");
	if (c & (1 << 17))
		printf("tce ");
	if (c & (1 << 21))
		printf("tbm ");
	if (c & (1 << 23))
		printf("PerfCtrExtCore ");
	if (c & (1 << 29))
		printf("monitorx ");
        printf("\n");

	/* Fn8000_0008_EBX Extended Feature Identifiers */
        __cpuid_count(0x80000008, 0, a, b, c, d);
        printf("CPUID.(EAX=8000_0008h,ECX=0) %lx %lx %lx %lx\n", a, b, c, d);
        if (b & (1 << 12))
                printf("ibpb ");
	if (b & (1 << 13))
		printf("wbinvd_int ");
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

	/* Fn8000_000Ah SVM Features */
	if (amd) {
        __cpuid_count(0x8000000A, 0, a, b, c, d);
        printf("CPUID.(EAX=8000_000ah,ECX=0) %lx %lx %lx %lx\n", a, b, c, d);
	printf("SvmRev %x\n", a & 0x0f);
	printf("NASID %x\n", b);
	if (d & (1 << 0))
		printf("NP ");
	if (d & (1 << 1))
		printf("LbrVirt ");
	if (d & (1 << 2))
		printf("SVML ");
	if (d & (1 << 3))
		printf("NRIPS ");
	if (d & (1 << 4))
		printf("TscRateMsr ");
	if (d & (1 << 5))
		printf("VmcbClean ");
	if (d & (1 << 6))
		printf("FlushByAsid ");
	if (d & (1 << 7))
		printf("DecodeAssists ");
	if (d & (1 << 8))
		printf("PmcVirt ");
	if (d & (1 << 10))
		printf("PauseFilter ");
	if (d & (1 << 12))
		printf("PauseFilterThreshold ");
	if (d & (1 << 13))
		printf("AVIC ");
	if (d & (1 << 15))
		printf("VMSAVEvirt ");
	if (d & (1 << 16))
		printf("VGIF ");
	if (d & (1 << 20))
		printf("SpecCtrl ");
	if (d & (1 << 21))
		printf("ROGPT ");
	if (d & (1 << 23))
		printf("HOST_MCE_OVERRIDE ");
	if (d & (1 << 24))
		printf("TlbiCtl ");
	if (d & (1 << 25))
		printf("VNMI ");
	if (d & (1 << 26))
		printf("IbsVirt ");
	if (d & (1 << 27))
		printf("ExtLvtAvicAccessChg ");
	if (d & (1 << 28))
		printf("NestedVirtVmcbAddrChk ");
	if (d & (1 << 29))
		printf("BusLockThreshold ");
	if (d & (1 << 30))
		printf("IdleHltIntercept ");
	printf("\n");
	}

	/* Fn8000_001A_EAX Performance Optimization Identifiers */
        __cpuid_count(0x8000001a, 0, a, b, c, d);
        printf("CPUID.(EAX=8000_001ah,ECX=0) %lx %lx %lx %lx\n", a, b, c, d);
	if (a & (1 << 0))
		printf("FP128 ");
	if (a & (1 << 1))
		printf("MOVU ");
	if (a & (1 << 2))
		printf("FP256 ");
	printf("\n");

	/* Fn8000_0021 Extended Feature Identification 2 */
        __cpuid_count(0x80000021, 0, a, b, c, d);
        printf("CPUID.(EAX=8000_0021h,ECX=0) %lx %lx %lx %lx\n", a, b, c, d);
	if (a & (1 << 0))
		printf("NoNestedDataBp ");
	if (a & (1 << 2))
		printf("LFenceAlwaysSerializing ");
	if (a & (1 << 5))
		printf("VERW_CLEAR ");
	if (a & (1 << 6))
		printf("NullSelectClearsBase ");
	if (a & (1 << 7))
		printf("UpperAddressIgnore ");
	if (a & (1 << 8))
		printf("AutomaticIBRS ");
	if (a & (1 << 10))
		printf("FastShortRepStosb ");
	if (a & (1 << 11))
		printf("FastShortRepeCmpsb ");
	if (a & (1 << 13))
		printf("PrefetchCtlMsr ");
	if (a & (1 << 15))
		printf("AMD_ERMSB ");
	if (a & (1 << 16))
		printf("OPCODE_0F017_RECLAIM ");
	if (a & (1 << 17))
		printf("CpuidUserDis ");
	if (a & (1 << 18))
		printf("EPSF ");
	if (a & (1 << 19))
		printf("FAST_REP_SCASB ");
	if (a & (1 << 20))
		printf("PREFETCHI ");
	if (a & (1 << 21))
		printf("FP512_DOWNGRADE ");
	if (a & (1 << 24))
		printf("ERAPS ");
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
	if (c & (1 << 1))
		printf("TSA_SQ_NO ");
	if (c & (1 << 2))
		printf("TSA_L1_NO ");
	printf("\n");

	/* Fn8000_0022 Extended Performance Monitoring and Debug */
        __cpuid_count(0x80000022, 0, a, b, c, d);
        printf("CPUID.(EAX=8000_0022h,ECX=0) %lx %lx %lx %lx\n", a, b, c, d);
	if (a & (1 << 0))
		printf("PerfMonV2 ");
	if (a & (1 << 1))
		printf("LbrStack ");
	if (a & (1 << 2))
		printf("LbrAndPmcFreeze ");
	printf("\n");
}
