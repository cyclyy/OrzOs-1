// main.c -- Defines the C-code kernel entry point, calls initialisation routines.

#include "sysdef.h"
#include "screen.h"
#include "interrupt.h"
#include "kmm.h"
#include "vmm.h"
#include "util.h"
#include "process.h"
#include "bootinfo.h"
#include "vfs.h"

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
    vfsMount("Boot",0,"tarfs",0,(void*)getBootInfo()->initrdAddr);
    DBG("AvailMem:%dKB",availMemory()/1024);

    for(;;);
    // never return here;
    return 0;
} 

void mount_root()
{
}

