// main.c -- Defines the C-code kernel entry point, calls initialisation routines.

#include "except.h"
#include "sysdef.h"
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
#include "pci.h"
#include "ide.h"
#include "cpu.h"
#include "cmos.h"
#include "debugdev.h"
#include "timer.h"
#include "libc/list.h"
#include "fs/ext2fs.h"

void cb(void *arg);
struct Timer *t;

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
    char *vram = (char*)((u64int)mib->PhysBasePtr);
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

void testE9()
{
}

u64int kmain(struct BootInfo *si)
{
    printk("Booting...\n");

    setBootInfo(si);
    initGlobalTimer();
    initInterrupt();
    initMemoryManagement();
    initTSS();
    initVFS();
    initSyscalls();
    initFpu();
    initCpuExceptions();
    //initIPC();
    initRTC();
    initRealModeInterface();
    vfsMount("Boot",0,"bootfs",0,(void*)PADDR_TO_VADDR(getBootInfo()->initrdAddr));
    vfsMount("Device",0,"devfs",0,0);
    i8042_Init();
    display_Init();
    debugdev_Init();
    pci_Init();
    ide_Init();
    ext2fs_Init();
    //testE9();
    vfsMount("C","Device:/Disk0Part0","ext2fs",0,0);
    struct VNode node;
    char buf[100];
    vfsOpen("C:/t1.txt",0,&node);
    vfsRead(&node, 100, buf);
    buf[99] = 0;
    printk("%s",buf);

    printk("InitAddr:%x\n", getBootInfo()->initrdAddr);
    printk("AvailMem:%dKB\n",availMemory()/1024);

    initMultitasking();
    initSoftTimer();
    kNewTask("Boot:/init", 0);

//    t = createTimer(1000, cb, (void*)1);
//    startTimer(t);

    rootTask();
    // never return here;
    return 0;
} 

void cb(void *arg)
{
    static long interval = 1;
    long i = (long)arg;
    if (interval > 64) {
        destroyTimer(t);
        return;
    }
    printk("Delayed call %d\n", interval);
    interval *= 2;
    t->expireTick = globalTicks + interval*1000;
    startTimer(t);
}
