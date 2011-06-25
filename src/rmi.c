#include "rmi.h"
#include "x86emu.h"
#include "util.h"

static X86EMU_pioFuncs pioFuncs = {
    .inb = &inb,
    .inw = &inw,
    .inl = &inl,
    .outb = &outb,
    .outw = &outw,
    .outl = &outl,
};

void initRealModeInterface()
{
    X86EMU_setupPioFuncs(&pioFuncs);
    memset(&M, 0, sizeof M);
    M.mem_base = KERNEL_HEAP_START_ADDR;
    M.mem_size = 0x100000;
}

void realModeInterrupt(u8int num)
{
    M.x86.R_SS = 0x6000;
    M.x86.R_ESP = 0x0000;
    M.x86.R_CS = 0x4000;
    M.x86.R_EIP = 0x01;
    *(u8 *)0x40001 = 0xf4; /* HLT, so the emulator knows where to stop */

    X86EMU_prepareForInt(num);
    X86EMU_exec();
}


// vim: sw=4 sts=4 et tw=100
