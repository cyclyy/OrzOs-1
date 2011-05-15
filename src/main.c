// main.c -- Defines the C-code kernel entry point, calls initialisation routines.

#include "sysdef.h"
#include "screen.h"
#include "dtable.h"
#include "mm.h"

struct StartupInfo {
    u64int memory;
    u64int initrdAddr;
    u64int initrdEnd;
    u64int freeMemStartAddr;
};

u64int kmain(struct StartupInfo *si)
{
    scr_clear();  
    scr_puts("Booting...\n");

    initIDT();
    initMemoryManagement(si->memory, si->freeMemStartAddr);

    for(;;);
    // never return here;
    return 0;
} 

void mount_root()
{
}

