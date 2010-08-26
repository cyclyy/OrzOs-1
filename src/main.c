// main.c -- Defines the C-code kernel entry point, calls initialisation routines.
// Made for JamesM's tutorials

#include "screen.h"
#include "desc_table.h"
#include "isr.h"
#include "timer.h"
#include "paging.h"
#include "kheap.h"
#include "vfs.h"
#include "multiboot.h"
#include "task.h"
#include "syscalls.h"
#include "fs/initrdfs.h"
#include "module.h"

u32int initial_esp;

multiboot_t *mboot_ptr;

void mount_root();

u32int kmain(multiboot_t *ptr, u32int esp)
{
    asm volatile("cli");

    scr_clear();  

    // All our initialisation calls will go in here.

    initial_esp = esp;

    mboot_ptr = ptr;

    placement_addr = *(u32int*)(mboot_ptr->mods_addr + 4);

    scr_puts("init_gdt.\n");
    init_gdt();

    scr_puts("init_idt.\n");
    init_idt();

    scr_puts("init_paging.\n");
    init_paging();

    scr_puts("init_timer.\n");
    init_timer(HZ);

    scr_puts("init_multitasking.\n");
    init_multitasking();

    asm volatile("sti");

    scr_puts("init_syscalls.\n");
    init_syscalls();

    scr_puts("init_vfs.\n");
    init_vfs();

    scr_puts("init_modules.\n");
    init_module_system(mboot_ptr->num, mboot_ptr->size, mboot_ptr->addr, mboot_ptr->shndx);

    scr_puts("load initrdfs module.\n");
    module_initrdfs_init();
    mount_root();

    load_module("/i8042.o");
    load_module("/pci.o");
    load_module("/ide.o"); 

    if (fork()) {
        while (1) 
            switch_task();
    } else {
        execve("/init",0,0);
    }

    // never return here;
    return 0;
} 

void mount_root()
{
    ASSERT(mboot_ptr->mods_count == 1);

    fs_driver_t *driver = get_fs_driver_byname("initrdfs");
    fs_t *fs = driver->createfs(driver, 0, 0, (void*) *(u32int*)mboot_ptr->mods_addr );
    vfs_mount_root(fs);
}

