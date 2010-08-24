#ifndef DESC_TABLE_H
#define DESC_TABLE_H

#include "common.h"

struct gdt_entry {
    u16int limit_low;
    u16int base_low;
    u8int base_middle;
    u8int access;
    u8int gran; // lower 4 bits of gran is higher 4 bits of limit;
    u8int base_high;
} __attribute__ ((packed));
typedef struct gdt_entry gdt_entry_t;

struct gdt_ptr {
    u16int limit;
    u32int base;
} __attribute__ ((packed));
typedef struct gdt_ptr gdt_ptr_t;

struct tss_entry_struct
{
    u32int prev_tss;   // The previous TSS - if we used hardware task switching this would form a linked list.
    u32int esp0;       // The stack pointer to load when we change to kernel mode.
    u32int ss0;        // The stack segment to load when we change to kernel mode.
    u32int esp1;       // Unused...
    u32int ss1;
    u32int esp2;  
    u32int ss2;   
    u32int cr3;   
    u32int eip;   
    u32int eflags;
    u32int eax;
    u32int ecx;
    u32int edx;
    u32int ebx;
    u32int esp;
    u32int ebp;
    u32int esi;
    u32int edi;
    u32int es;         // The value to load into ES when we change to kernel mode.
    u32int cs;         // The value to load into CS when we change to kernel mode.
    u32int ss;         // The value to load into SS when we change to kernel mode.
    u32int ds;         // The value to load into DS when we change to kernel mode.
    u32int fs;         // The value to load into FS when we change to kernel mode.
    u32int gs;         // The value to load into GS when we change to kernel mode.
    u32int ldt;        // Unused...
    u16int trap;
    u16int iomap_base;
} __attribute__((packed));

typedef struct tss_entry_struct tss_entry_t;


// gdt_flush(&gdt_entry_t)
extern void gdt_flush(u32int);

void init_gdt();

// =========================================================================

struct idt_entry {
    u16int offset_low;
    u16int selector;
    u8int zero;
    u8int type_attr;
    u16int offset_high;
} __attribute__ ((packed));
typedef struct idt_entry idt_entry_t;

struct idt_ptr {
    u16int limit;
    u32int base;
} __attribute__ ((packed));
typedef struct idt_ptr idt_ptr_t;

// gdt_flush(&gdt_entry_t)
extern void idt_flush(u32int);

void init_idt();

void set_kernel_stack(u32int esp);

#endif

