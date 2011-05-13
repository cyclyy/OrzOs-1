// main.c -- Defines the C-code kernel entry point, calls initialisation routines.

#include "types.h"
#include "screen.h"

u64int initial_esp;

void mount_root();

u64int kmain()
{
    asm volatile("cli");

    scr_clear();  
    scr_puts("Booting...\n");
    for(;;);
    // never return here;
    return 0;
} 

void mount_root()
{
}

