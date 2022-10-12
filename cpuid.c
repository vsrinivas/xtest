#include <cpuid.h>

int main(int argc, char *argv[]) {
        unsigned long a, b, c, d;

        __cpuid_count(0x80000008, 0, a, b, c, d);
        printf("Fn8000_0008 %x %x %x %x\n", a, b, c, d);

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

        printf("\n");
}
