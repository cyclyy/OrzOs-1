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
#include "video/display.h"
#include "video/vbe.h"

void testVBE()
{
    int ret;
    struct VBEInfoBlock *ib;
    struct VBEModeInfoBlock *mib;

    /* detect presence of vbe2+ */
    printk("Detecting presence of VBE2...\n");
    ib = (struct VBEInfoBlock *)realModeAlloc(sizeof(struct VBEInfoBlock));
    ret = getVBEInfo(ib);
    printk("Result: %x\n", ret);
    if ((M.x86.R_EAX & 0x00ff) != 0x4f) {
        printk("VBE not supported\n");
    }
    if ((M.x86.R_EAX & 0xff00) != 0) {
        printk("VBE call failed: %x\n", M.x86.R_EAX & 0xffff);
    }
    mib = (struct VBEModeInfoBlock *)realModeAlloc(sizeof(struct VBEModeInfoBlock));
    getVBEModeInfo(0x115, mib);
    printk("Framebuffer at %x, width: %d, height: %d\n", mib->PhysBasePtr, mib->XResolution, mib->YResolution);
    setVBEMode(0x115);
    char *vram = (char*)(mib->PhysBasePtr);
    int i,j,k;
    for (i=0; i< mib->YResolution; i++) {
        for (j=0; j< mib->XResolution; j++) {
            k = 3 * (i * mib->XResolution + j); 
            vram[k] = i * 255 / mib->YResolution;
            vram[k+1] = i * 255 / mib->YResolution;
            vram[k+2] = i * 255 / mib->YResolution;
        }
    }
    setVBEMode(0x3);
    printk("Switch: %x\n", M.x86.R_EAX);
}

u64int kmain(struct BootInfo *si)
{
    scr_clear();  
    scr_puts("Booting...\n");

    setBootInfo(si);
    initInterrupt();
    initMemoryManagement(si->memory, si->freeMemStartAddr);
    initTSS();
    initVFS();
    initSyscalls();
    initCpuExceptions();
    initIPC();
    vfsMount("Boot",0,"bootfs",0,(void*)PADDR_TO_VADDR(getBootInfo()->initrdAddr));
    vfsMount("Device",0,"devfs",0,0);
    initRealModeInterface();
    i8042_Init();
    display_Init();
    //testVBE();

    printk("InitAddr:%x\n", getBootInfo()->initrdAddr);
    printk("AvailMem:%dKB\n",availMemory()/1024);

    initMultitasking();
    kNewTask("Boot:/init", 0);

    rootTask();
    // never return here;
    return 0;
} 

