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

    DBG("InitAddr:%x", getBootInfo()->initrdAddr);
    DBG("AvailMem:%dKB",availMemory()/1024);

    kNewTask("Boot:/init", 0);

    rootTask();
    // never return here;
    return 0;
} 

void mount_root()
{
}

