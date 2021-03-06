#ifndef MULTIBOOT_H
#define MULTIBOOT_H 

#include "common.h"

typedef struct multiboot {
    u32int flags;
    u32int mem_lower;
    u32int mem_upper;
    u32int boot_device;
    u32int cmdline;
    u32int mods_count;
    u32int mods_addr;
    u32int num;
    u32int size;
    u32int addr;
    u32int shndx;
    u32int mmap_length;
    u32int mmap_addr;
    u32int drives_length;
    u32int drives_addr;
    u32int config_table;
    u32int boot_loader_name;
    u32int apm_table;
    u32int vbe_control_info;
    u32int vbe_mode_info;
    u32int vbe_mode;
    u32int vbe_interface_seg;
    u32int vbe_interface_off;
    u32int vbe_interface_len;    
} multiboot_t;

struct multiboot_mod_list
{
    /* the memory used goes from bytes 'mod_start' to 'mod_end-1' inclusive */
    u32int mod_start;
    u32int mod_end;

    /* Module command line */
    u32int cmdline;

    /* padding to take it to 16 bytes (must be zero) */
    u32int pad;
};
typedef struct multiboot_mod_list multiboot_module_t;


#endif /* MULTIBOOT_H */
