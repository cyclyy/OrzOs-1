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
#include "rmi.h"
#include "video/vbe.h"

void testVBE()
{
    int ret;
    struct VBEInfoBlock *ib;
    struct VBEModeInfoBlock *mib;

    /* detect presence of vbe2+ */
    printk("Detecting presence of VBE2...\n");
    ib = (struct VBEInfoBlock *)0xB000;
    ret = getVBEInfo(ib);

    printk("Result: %x\n", ret);

    if ((M.x86.R_EAX & 0x00ff) != 0x4f) {
        printk("VBE not supported\n");
    }

    if ((M.x86.R_EAX & 0xff00) != 0) {
        printk("VBE call failed: %x\n", M.x86.R_EAX & 0xffff);
    }

    u16int mode = 0x115;
    mib = (struct VBEModeInfoBlock *)0x10000;
    getVBEModeInfo(mode, mib);

    printk("Framebuffer at %x, width: %d, height: %d\n", mib->PhysBasePtr, mib->XResolution, mib->YResolution);

    setVBEMode(mode);
    printk("Switch: %x\n", M.x86.R_EAX);
    getVBECurrentMode(&mode);
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
    initRealModeInterface();
    //testVBE();

    printk("InitAddr:%x\n", getBootInfo()->initrdAddr);
    printk("AvailMem:%dKB\n",availMemory()/1024);

    kNewTask("Boot:/init", 0);

    rootTask();
    // never return here;
    return 0;
} 

