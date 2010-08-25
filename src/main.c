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

void init_modules();
void mount_root();
void dump_initrd();

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
} 

void dump_initrd()
{
    vnode_t *dir = 0;
    u32int i = 0;
//    scr_putp("fs_root",fs_root);
    while ((dir = vfs_readdir(vfs_root, i++))) {
        scr_puts("name:");
        scr_puts(dir->name);
        scr_puts("\n");

        vnode_t *node = vfs_finddir(vfs_root, dir->name);

        ASSERT(node);

        if (node->flags & VFS_FILE) {
            scr_puts("\t vnode file type,content is \"");
            u8int *buf;
            buf = (u8int*)kmalloc(node->length);
            vfs_read(node,0,node->length,buf);
            scr_puts((const char*)buf);
            kfree(buf);
            scr_puts("\"\n");
        } else {
            scr_puts("\t vnode dir type.");
            scr_puts("\n");
        }
    }

}

void init_modules()
{
}

void mount_root()
{
    ASSERT(mboot_ptr->mods_count == 1);

    fs_driver_t *driver = get_fs_driver_byname("initrdfs");
    ASSERT(driver);
    fs_t *root_fs = driver->createfs(driver, 0, 0, (void*) *(u32int*)mboot_ptr->mods_addr );
    ASSERT(root_fs);
    vfs_mount(vfs_root, root_fs);
}

