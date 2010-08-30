// main.c -- Defines the C-code kernel entry point, calls initialisation routines.

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
#include "fs/ramfs.h"
#include "module.h"
#include "cpio.h"

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

    scr_puts("load ramfs module.\n");
    module_ramfs_init();
    mount_root();

    vfs_mkdir("/dev",0);

    load_module("/boot/modules/i8042.o");
    load_module("/boot/modules/pci.o");
    load_module("/boot/modules/ide.o"); 
    load_module("/boot/modules/ext2fs.o"); 

    fs_driver_t *driver = get_fs_driver_byname("ext2fs");
    driver->fs_drv_ops->init(driver);
    fs_t *fs = driver->fs_drv_ops->createfs(driver, "/dev/part", 0, 0);
    vfs_mkdir("/volumn",0);
    vfs_mkdir("/volumn/c",0);
    vfs_mount("/volumn/c",fs);

    if (fork()) {
        while (1) 
            switch_task();
    } else {
        execve("/boot/init",0,0);
    }

    // never return here;
    return 0;
} 

void mount_root()
{
    ASSERT(mboot_ptr->mods_count >= 1);

    fs_driver_t *driver = get_fs_driver_byname("ramfs");
    driver->fs_drv_ops->init(driver);
    fs_t *fs = driver->fs_drv_ops->createfs(driver, 0, 0, 0);
    vfs_mount_root(fs);

    multiboot_module_t *m = (multiboot_module_t*)mboot_ptr->mods_addr;
    printk("mboot mod_start %p, mod_end %p, cmdline %s, pad %p \n", 
            m->mod_start,
            m->mod_end,
            m->cmdline,
            m->pad);
    extract_cpio((u8int*)m->mod_start, m->mod_end-m->mod_start);
}

