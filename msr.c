#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// ./msr {value of IA32_ARCH_CAPABILITIES (10ah)
int main(int argc, char *argv[]) {
	uint64_t val;

	val = strtol(argv[1], NULL, 16);
	printf("%llx\n", val);
	if (val & (1 << 0))
		printf("RDCL_NO ");
	if (val & (1 << 1))
		printf("IBRS_ALL ");
	if (val & (1 << 2))
		printf("RSBA ");
	if (val & (1 << 3))
		printf("SKIP_L1DFL_VMENTRY ");
	if (val & (1 << 4))
		printf("SSB_NO ");
	if (val & (1 << 5))
		printf("MDS_NO ");
	if (val & (1 << 6))
		printf("IF_PSCHANGE_MC_NO ");
	if (val & (1 << 7))
		printf("TSX_CTRL ");
	if (val & (1 << 8))
		printf("TAA_NO ");
	if (val & (1 << 10))
		printf("MISC_PACKAGE_CTRLS ");
	if (val & (1 << 11))
		printf("ENERGY_FILTERING_CTL ");
	if (val & (1 << 12))
		printf("DOITM ");
	if (val & (1 << 13))
		printf("SBDR_SSDP_NO ");
	if (val & (1 << 14))
		printf("FBSDP_NO ");
	if (val & (1 << 15))
		printf("PSDP_NO ");
	if (val & (1 << 16))
		printf("MCU_Enumeration ");
	if (val & (1 << 17))
		printf("FB_CLEAR ");
	if (val & (1 << 18))
		printf("FB_CLEAR_CTRL ");
	if (val & (1 << 19))
		printf("RRSBA ");
	if (val & (1 << 20))
		printf("BHI_NO ");
	if (val & (1 << 21))
		printf("XAPIC_DISABLE_STATUS ");
	if (val & (1 << 23))
		printf("OVERCLOCKING_STATUS ");
	if (val & (1 << 24))
		printf("PBRSB_NO ");
	if (val & (1 << 25))
		printf("GDS_CTRL ");
	if (val & (1 << 26))
		printf("GDS_NO ");
	if (val & (1 << 27))
		printf("RFDS_NO ");
	if (val & (1 << 28))
		printf("RFDS_CLEAR ");
	if (val & (1 << 29))
		printf("IGN_UMONITOR_SUPPORT ");
	if (val & (1 << 30))
		printf("MON_UMON_MITG_SUPPORT ");

	printf("\n");
}
