#include "cpu.h"
#include "sysdef.h"

static int cpuidFeatureTested = 0;
static u32int cpuidEax, cpuidEbx, cpuidEcx, cpuidEdx;

int hasCpuFeature(int ecxFeat, int edxFeat)
{
    int ok;
    if (!cpuidFeatureTested) {
        getCpuId(CPUID_GETFEATURES, &cpuidEax, &cpuidEbx, &cpuidEcx, &cpuidEdx);
    }
    ok = 0;
    ok |= cpuidEcx & ecxFeat;
    ok |= cpuidEdx & edxFeat;
    return ok;
}

void getCpuId(u32int code, u32int *a, u32int *b, u32int *c, u32int *d)
{
    __asm__ __volatile__("cpuid":"=a"(*a),"=b"(*b),"=c"(*c),"=d"(*d):"0"(code));
}

void setFpuControlWord(u16int cw)
{
    if (hasCpuFeature(0, CPUID_FEAT_EDX_FPU)) {
        // FLDCW = Load FPU Control Word
        asm volatile("fldcw %0;    "    // sets the FPU control word to "cw"
                ::"m"(cw)); 
    }
}


void initFpu()
{
    u64int cr4;
    if (hasCpuFeature(0, CPUID_FEAT_EDX_FPU)) {
        __asm__ __volatile__("mov %%cr4, %0;":"=r"(cr4));

        // set OSFXSR (bit 9)
        cr4 |= 0x200;
        // reload CR4 and INIT the FPU (FINIT)
        __asm__ __volatile__("mov %0, %%cr4; finit;" : : "r"(cr4));
        // set the FPU Control Word
        setFpuControlWord(0x37F);
    }
}

void initSse()
{
    u64int cr0, cr4;
    __asm__ __volatile__("mov %%cr0, %0; mov %%cr4, %1;":"=r"(cr0),"=r"(cr4));
    // clear CR0.EM (bit 2)
    cr0 &= ~(1 << 2);
    // set CR0.MP (bit 1)
    cr0 |= (1 << 1);
    // set CR4.OSFXSR (bit 9)
    cr4 |= (1 << 9);
    // set CR4.OSXMMEXCPT (bit 10)
    cr4 |= (1 << 10);
    __asm__ __volatile__("mov %0, %%cr0; mov %1, %%cr4;"::"r"(cr0),"r"(cr4));
}
// vim: sw=4 sts=4 et tw=100
