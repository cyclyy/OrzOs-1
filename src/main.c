// main.c -- Defines the C-code kernel entry point, calls initialisation routines.

#include "except.h"
#include "sysdef.h"
#include "screen.h"
#include "interrupt.h"
#include "kmm.h"
#include "vmm.h"
#include "util.h"
#include "task.h"
#include "bootinfo.h"
#include "vfs.h"
#include "paging.h"
#include "ksyscall.h"
#include "message.h"
#include "except.h"
#include "i8042.h"
#include "x86emu.h"

static void bios_interrupt(unsigned char num, X86EMU_regs *regs)
{
    M.x86 = *regs;
    /* Mmmm, fixed addresses */
    M.x86.R_SS = 0x8000;
    M.x86.R_ESP = 0x0000;
    M.x86.R_CS = 0x4000;
    M.x86.R_EIP = 0x01;
    *(u8 *)0x40001 = 0xf4; /* HLT, so the emulator knows where to stop */

    X86EMU_prepareForInt(num);
    X86EMU_exec();

    *regs = M.x86;
}

void testVBE()
{
    memset(&M, 0, sizeof M);
    M.mem_base = HEAP_START_ADDR;
    M.mem_size = 0x100000;
    X86EMU_regs regs;

    /* detect presence of vbe2+ */
    char *buffer = (char *)0xB000;
    memset(&regs, 0, sizeof regs);
    regs.R_EAX = 0x4f00;
    buffer[0] = 'V';
    buffer[1] = 'B';
    buffer[2] = 'E';
    buffer[3] = '2';
    regs.R_ES = 0;
    regs.R_EDI = 0xB000;

    printk("Detecting presence of VBE2...\n");

    bios_interrupt(0x10, &regs);

    printk("Result: %04x\n", regs.R_EAX);

    if ((regs.R_EAX & 0x00ff) != 0x4f) {
        printk("VBE not supported\n");
    }

    if ((regs.R_EAX & 0xff00) != 0) {
        printk("VBE call failed: %x\n", regs.R_EAX & 0xffff);
    }

    u16int mode = 0x115;

    /* get mode info */
    memset(&regs, 0, sizeof regs);
    regs.R_EAX = 0x4f01;
    regs.R_ES = 0x1000;
    regs.R_EDI = 0x0000;
    regs.R_ECX = mode;

    bios_interrupt(0x10, &regs);

    buffer = 0x10000;
    unsigned long frame_buffer = *((unsigned long *)(buffer+0x28));
    unsigned short width = *((unsigned short *)(buffer+0x12));
    unsigned short height = *((unsigned short *)(buffer+0x14));

    printk("Framebuffer at %x, width: %d, height: %d\n", frame_buffer, width, height);

    /* set the mode */
    memset(&regs, 0, sizeof regs);
    regs.R_EAX = 0x4f02;
    if(mode) {
        /* Use linear framebuffer model */
        regs.R_EBX = mode | (1<<14);
    } else {
        regs.R_EBX = 3;
    }

    bios_interrupt(0x10, &regs);

    printk("Switch: %04x\n", regs.R_EAX);

}

u64int kmain(struct BootInfo *si)
{
    scr_clear();  
    scr_puts("Booting...\n");

    setBootInfo(si);
    initInterrupt();
    initMemoryManagement(si->memory, si->freeMemStartAddr);
    initTSS();
    initMultitasking();
    initVFS();
    initSyscalls();
    initCpuExceptions();
    initIPC();
    vfsMount("Boot",0,"bootfs",0,(void*)PADDR_TO_VADDR(getBootInfo()->initrdAddr));
    vfsMount("Device",0,"devfs",0,0);
    i8042_Init();
    testVBE();

    DBG("InitAddr:%x", getBootInfo()->initrdAddr);
    DBG("AvailMem:%dKB",availMemory()/1024);

    kNewTask("Boot:/init", 0);

    rootTask();
    // never return here;
    return 0;
} 

