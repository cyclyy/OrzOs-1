#ifndef DTABLE_H
#define DTABLE_H

#include "sysdef.h"

#define IO_BITMAP_BITS  65536
#define IO_BITMAP_BYTES (IO_BITMAP_BITS/8)
#define IO_BITMAP_LONGS (IO_BITMAP_BYTES/sizeof(u64int))

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

void initIDT();

//void set_kernel_stack(u32int esp);

struct TSSEntry {
    u16int limitLow;
    u16int baseLow;
    u8int baseMiddle;
    u8int typeAttr;
    u8int limitMiddle;
    u8int baseMiddle2;
    u32int baseHigh;
    u32int reserved;
} __attribute__ ((packed));

struct TSS {
        u32int reserved1;
        u64int rsp0;       
        u64int rsp1;
        u64int rsp2;
        u64int reserved2;
        u64int ist[7];
        u32int reserved3;
        u32int reserved4;
        u16int reserved5;
        u16int ioBitmapBase;
        /*
         * The extra 1 is there because the CPU will access an
         * additional byte beyond the end of the IO permission
         * bitmap. The extra byte must be all 1 bits, and must
         * be within the limit. Thus we have:
         *
         * 128 bytes, the bitmap itself, for ports 0..0x3ff
         * 8 bytes, for an extra "long" of ~0UL
         */
        u64int io_bitmap[IO_BITMAP_LONGS + 1];
} __attribute__((packed));

struct TSSPtr {
    u16int limit;
    u64int base;
} __attribute__ ((packed));

void initTSS();

void setKernelStack(u64int rsp);

#endif /* DTABLE_H */

