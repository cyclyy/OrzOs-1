#include "dtable.h"
#include "util.h"

/*
gdt_entry_t gdt_entries[5];

gdt_ptr_t gdt_ptr;
*/

struct IDTEntry IDTTable[256];

struct IDTPtr tr;

//tss_entry_t tss0;

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();
extern void isr128();

extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();

/*
void setGDT(GDTEntry  *entry, u32int base, u32int limit, u8int access, u8int granunarity)
{
    if (entry) {
        entry->limit_low = limit & 0xffff;
        entry->base_low = base & 0xffff;
        entry->base_middle = (base & 0xff0000) >> 16;
        entry->access = access;
        entry->gran = (limit >> 16) & 0x0f;
        entry->gran |= gran & 0xf0;
        entry->base_high = (base & 0xff000000) >> 24;
    }
}
*/

void setIDT(struct IDTEntry *entry, void* offaddr, u16int selector, u8int typeAttr)
{
    u64int offset = (u64int)offaddr;
    if (entry) {
        entry->offsetLow = offset & 0xffff;
        entry->selector = selector;
        entry->typeAttr = typeAttr | 0x60;
        entry->zero = 0;
        entry->offsetMiddle = (offset & 0xffff0000) >> 16;
        entry->offsetHigh = (offset & 0xffffffff00000000) >> 32;
    }
}

/*
void write_tss()
{
    memset(&tss0, 0, sizeof(tss_entry_t));
    tss0.ss0 = 0x10;
    tss0.cs = 0x0b;
    tss0.ds = tss0.es = tss0.fs = tss0.gs = 0x13;

    u32int addr = (u32int)&tss0;
    gdt_set_entry(&gdt_entries[5], addr, addr+sizeof(tss_entry_t), 0xe9, 0xc0);
}

void init_gdt()
{
    gdt_ptr.limit = (sizeof(gdt_entry_t) * 6) - 1;
    gdt_ptr.base = (u32int)&gdt_entries;


    gdt_set_entry(&gdt_entries[0],0,0,0,0);
    gdt_set_entry(&gdt_entries[1],0,0xffffffff,0x9a,0xcf);
    gdt_set_entry(&gdt_entries[2],0,0xffffffff,0x92,0xcf);
    gdt_set_entry(&gdt_entries[3],0,0xffffffff,0xfa,0xcf);
    gdt_set_entry(&gdt_entries[4],0,0xffffffff,0xf2,0xcf);
    write_tss();

    gdt_flush( (u32int)&gdt_ptr );

    tss_flush();
}
*/

void initIDT()
{
    tr.limit = sizeof(struct IDTEntry) * 256 - 1;
    tr.base = (u64int) &IDTTable;

    memset(IDTTable, 0, sizeof(struct IDTEntry)*256);

    setIDT(&IDTTable[0], (void*)isr0, 0x08, 0x8e);
    setIDT(&IDTTable[1], (void*)isr1, 0x08, 0x8e);
    setIDT(&IDTTable[2], (void*)isr2, 0x08, 0x8e);
    setIDT(&IDTTable[3], (void*)isr3, 0x08, 0x8e);
    setIDT(&IDTTable[4], (void*)isr4, 0x08, 0x8e);
    setIDT(&IDTTable[5], (void*)isr5, 0x08, 0x8e);
    setIDT(&IDTTable[6], (void*)isr6, 0x08, 0x8e);
    setIDT(&IDTTable[7], (void*)isr7, 0x08, 0x8e);
    setIDT(&IDTTable[8], (void*)isr8, 0x08, 0x8e);
    setIDT(&IDTTable[9], (void*)isr9, 0x08, 0x8e);
    setIDT(&IDTTable[10], (void*)isr10, 0x08, 0x8e);
    setIDT(&IDTTable[11], (void*)isr11, 0x08, 0x8e);
    setIDT(&IDTTable[12], (void*)isr12, 0x08, 0x8e);
    setIDT(&IDTTable[13], (void*)isr13, 0x08, 0x8e);
    setIDT(&IDTTable[14], (void*)isr14, 0x08, 0x8e);
    setIDT(&IDTTable[15], (void*)isr15, 0x08, 0x8e);
    setIDT(&IDTTable[16], (void*)isr16, 0x08, 0x8e);
    setIDT(&IDTTable[17], (void*)isr17, 0x08, 0x8e);
    setIDT(&IDTTable[18], (void*)isr18, 0x08, 0x8e);
    setIDT(&IDTTable[19], (void*)isr19, 0x08, 0x8e);
    setIDT(&IDTTable[20], (void*)isr20, 0x08, 0x8e);
    setIDT(&IDTTable[21], (void*)isr21, 0x08, 0x8e);
    setIDT(&IDTTable[22], (void*)isr22, 0x08, 0x8e);
    setIDT(&IDTTable[23], (void*)isr23, 0x08, 0x8e);
    setIDT(&IDTTable[24], (void*)isr24, 0x08, 0x8e);
    setIDT(&IDTTable[25], (void*)isr25, 0x08, 0x8e);
    setIDT(&IDTTable[26], (void*)isr26, 0x08, 0x8e);
    setIDT(&IDTTable[27], (void*)isr27, 0x08, 0x8e);
    setIDT(&IDTTable[28], (void*)isr28, 0x08, 0x8e);
    setIDT(&IDTTable[29], (void*)isr29, 0x08, 0x8e);
    setIDT(&IDTTable[30], (void*)isr30, 0x08, 0x8e);
    setIDT(&IDTTable[31], (void*)isr31, 0x08, 0x8e);
    setIDT(&IDTTable[128], (void*)isr128, 0x08, 0x8e);

    u8int a1, b1;

    // save mask
    a1 = inb(0x21);
    b1 = inb(0xa1);

    // send ICW1
    outb(0x20,0x11);
    outb(0xa0,0x11);

    // send ICW2
    outb(0x21,0x20);
    outb(0xa1,0x28);

    // send ICW3
    outb(0x21,4);
    outb(0xa1,2);

    // send ICW4
    outb(0x21,1);
    outb(0xa1,1);

    // restore mask
    outb(0x21,a1);
    outb(0xa1,b1);

    setIDT(&IDTTable[32], (void*)irq0, 0x08, 0x8e);
    setIDT(&IDTTable[33], (void*)irq1, 0x08, 0x8e);
    setIDT(&IDTTable[34], (void*)irq2, 0x08, 0x8e);
    setIDT(&IDTTable[35], (void*)irq3, 0x08, 0x8e);
    setIDT(&IDTTable[36], (void*)irq4, 0x08, 0x8e);
    setIDT(&IDTTable[37], (void*)irq5, 0x08, 0x8e);
    setIDT(&IDTTable[38], (void*)irq6, 0x08, 0x8e);
    setIDT(&IDTTable[39], (void*)irq7, 0x08, 0x8e);
    setIDT(&IDTTable[40], (void*)irq8, 0x08, 0x8e);
    setIDT(&IDTTable[41], (void*)irq9, 0x08, 0x8e);
    setIDT(&IDTTable[42], (void*)irq10, 0x08, 0x8e);
    setIDT(&IDTTable[43], (void*)irq11, 0x08, 0x8e);
    setIDT(&IDTTable[44], (void*)irq12, 0x08, 0x8e);
    setIDT(&IDTTable[45], (void*)irq13, 0x08, 0x8e);
    setIDT(&IDTTable[46], (void*)irq14, 0x08, 0x8e);

    flushIDT(&tr);
    //asm volatile("sti");
}

/*
void set_kernel_stack(u32int esp)
{
    tss0.esp0 = esp;
}
*/

