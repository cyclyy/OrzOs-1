#include "desc_table.h"
#include "common.h"

gdt_entry_t gdt_entries[5];

gdt_ptr_t gdt_ptr;

idt_entry_t idt_entries[256];

idt_ptr_t idt_ptr;

tss_entry_t tss0;

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

void gdt_set_entry(gdt_entry_t *entry, u32int base, u32int limit, u8int access, u8int gran)
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

void idt_set_entry(idt_entry_t *entry, u32int offset, u16int selector, u8int type_attr)
{
    if (entry) {
        entry->offset_low = offset & 0xffff;
        entry->selector = selector;
        entry->type_attr = type_attr | 0x60;
        entry->zero = 0;
        entry->offset_high = (offset & 0xffff0000) >> 16;
    }
}

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

void init_idt()
{
    idt_ptr.limit = sizeof(idt_entry_t) * 256 - 1;
    idt_ptr.base = (u32int) &idt_entries;

    memset(idt_entries, 0, sizeof(idt_entry_t)*256);

    idt_set_entry(&idt_entries[0], (u32int)isr0, 0x08, 0x8e);
    idt_set_entry(&idt_entries[1], (u32int)isr1, 0x08, 0x8e);
    idt_set_entry(&idt_entries[2], (u32int)isr2, 0x08, 0x8e);
    idt_set_entry(&idt_entries[3], (u32int)isr3, 0x08, 0x8e);
    idt_set_entry(&idt_entries[4], (u32int)isr4, 0x08, 0x8e);
    idt_set_entry(&idt_entries[5], (u32int)isr5, 0x08, 0x8e);
    idt_set_entry(&idt_entries[6], (u32int)isr6, 0x08, 0x8e);
    idt_set_entry(&idt_entries[7], (u32int)isr7, 0x08, 0x8e);
    idt_set_entry(&idt_entries[8], (u32int)isr8, 0x08, 0x8e);
    idt_set_entry(&idt_entries[9], (u32int)isr9, 0x08, 0x8e);
    idt_set_entry(&idt_entries[10], (u32int)isr10, 0x08, 0x8e);
    idt_set_entry(&idt_entries[11], (u32int)isr11, 0x08, 0x8e);
    idt_set_entry(&idt_entries[12], (u32int)isr12, 0x08, 0x8e);
    idt_set_entry(&idt_entries[13], (u32int)isr13, 0x08, 0x8e);
    idt_set_entry(&idt_entries[14], (u32int)isr14, 0x08, 0x8e);
    idt_set_entry(&idt_entries[15], (u32int)isr15, 0x08, 0x8e);
    idt_set_entry(&idt_entries[16], (u32int)isr16, 0x08, 0x8e);
    idt_set_entry(&idt_entries[17], (u32int)isr17, 0x08, 0x8e);
    idt_set_entry(&idt_entries[18], (u32int)isr18, 0x08, 0x8e);
    idt_set_entry(&idt_entries[19], (u32int)isr19, 0x08, 0x8e);
    idt_set_entry(&idt_entries[20], (u32int)isr20, 0x08, 0x8e);
    idt_set_entry(&idt_entries[21], (u32int)isr21, 0x08, 0x8e);
    idt_set_entry(&idt_entries[22], (u32int)isr22, 0x08, 0x8e);
    idt_set_entry(&idt_entries[23], (u32int)isr23, 0x08, 0x8e);
    idt_set_entry(&idt_entries[24], (u32int)isr24, 0x08, 0x8e);
    idt_set_entry(&idt_entries[25], (u32int)isr25, 0x08, 0x8e);
    idt_set_entry(&idt_entries[26], (u32int)isr26, 0x08, 0x8e);
    idt_set_entry(&idt_entries[27], (u32int)isr27, 0x08, 0x8e);
    idt_set_entry(&idt_entries[28], (u32int)isr28, 0x08, 0x8e);
    idt_set_entry(&idt_entries[29], (u32int)isr29, 0x08, 0x8e);
    idt_set_entry(&idt_entries[30], (u32int)isr30, 0x08, 0x8e);
    idt_set_entry(&idt_entries[31], (u32int)isr31, 0x08, 0x8e);
    idt_set_entry(&idt_entries[128], (u32int)isr128, 0x08, 0x8e);

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

    idt_set_entry(&idt_entries[32], (u32int)irq0, 0x08, 0x8e);
    idt_set_entry(&idt_entries[33], (u32int)irq1, 0x08, 0x8e);
    idt_set_entry(&idt_entries[34], (u32int)irq2, 0x08, 0x8e);
    idt_set_entry(&idt_entries[35], (u32int)irq3, 0x08, 0x8e);
    idt_set_entry(&idt_entries[36], (u32int)irq4, 0x08, 0x8e);
    idt_set_entry(&idt_entries[37], (u32int)irq5, 0x08, 0x8e);
    idt_set_entry(&idt_entries[38], (u32int)irq6, 0x08, 0x8e);
    idt_set_entry(&idt_entries[39], (u32int)irq7, 0x08, 0x8e);
    idt_set_entry(&idt_entries[40], (u32int)irq8, 0x08, 0x8e);
    idt_set_entry(&idt_entries[41], (u32int)irq9, 0x08, 0x8e);
    idt_set_entry(&idt_entries[42], (u32int)irq10, 0x08, 0x8e);
    idt_set_entry(&idt_entries[43], (u32int)irq11, 0x08, 0x8e);
    idt_set_entry(&idt_entries[44], (u32int)irq12, 0x08, 0x8e);
    idt_set_entry(&idt_entries[45], (u32int)irq13, 0x08, 0x8e);
    idt_set_entry(&idt_entries[46], (u32int)irq14, 0x08, 0x8e);

    idt_flush((u32int)&idt_ptr);
}

void set_kernel_stack(u32int esp)
{
    tss0.esp0 = esp;
}

