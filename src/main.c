// main.c -- Defines the C-code kernel entry point, calls initialisation routines.

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
    DBG("InitAddr:%x", getBootInfo()->initrdAddr);
    DBG("AvailMem:%dKB",availMemory()/1024);
    vfsMount("Boot",0,"bootfs",0,(void*)PADDR_TO_VADDR(getBootInfo()->initrdAddr));
    struct VNode node;
    vfsOpen("Boot:/a.txt",0,&node);
    char buf[1000];
    s64int n;
    n = vfsRead(&node, 0, 1000, buf);
    printk(buf);

    kNewTask("Boot:/init", 0);

    rootTask();
    // never return here;
    return 0;
} 

void mount_root()
{
}

