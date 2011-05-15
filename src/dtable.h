#ifndef DTABLE_H
#define DTABLE_H

#include "sysdef.h"

struct GDTEntry {
    u16int limitLow;
    u16int baseLow;
    u8int baseMiddle;
    u8int access;
    u8int granunarity; // lower 4 bits of gran is higher 4 bits of limit;
    u8int baseHigh;
} __attribute__ ((packed));

struct GDTPtr {
    u16int limit;
    u64int base;
} __attribute__ ((packed));

// gdt_flush(&gdt_entry_t)
extern void gdtFlush(u64int);

void initGdt();

// =========================================================================

struct IDTEntry {
    u16int offsetLow;
    u16int selector;
    u8int zero;
    u8int typeAttr;
    u16int offsetMiddle;
    u32int offsetHigh;
    u32int reserved;
} __attribute__ ((packed));

struct IDTPtr {
    u16int limit;
    u64int base;
} __attribute__ ((packed));

// gdt_flush(&gdt_entry_t)
extern void idt_flush(u32int);

void initIdt();

//void set_kernel_stack(u32int esp);


#endif

