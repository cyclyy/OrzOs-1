#include "dtable.h"
#include "sysdef.h"
#include "util.h"

struct GDTPtr   gdtPtr;
struct GDTEntry gdtTable[8];
struct IDTEntry idtTable[256];

struct IDTPtr idtPtr;

struct TSS tss0;

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

extern void flushGDT(struct GDTPtr *);
extern void flushIDT();
extern void flushTSS();

void setGDT(struct GDTEntry *entry, u32int base, u32int limit, u8int access, u8int granunarity)
{
    if (entry) {
        entry->limitLow = limit & 0xffff;
        entry->baseLow = base & 0xffff;
        entry->baseMiddle = (base & 0xff0000) >> 16;
        entry->access = access;
        entry->granunarity = 0x20;
        entry->baseHigh = (base & 0xff000000) >> 24;
    }
}  

void setTSS(struct GDTEntry *entry, u64int base, u64int limit)
{
    struct TSSEntry *tssEntry = (struct TSSEntry *)entry;
    memset(tssEntry,0,sizeof(struct TSSEntry));

    tssEntry->limitLow = limit & 0xffff;
    tssEntry->baseLow = base & 0xffff;
    tssEntry->baseMiddle = (base >> 16) & 0xff;
    tssEntry->typeAttr = 0xe9;
    tssEntry->limitMiddle = (limit >> 16) & 0x0f;
    tssEntry->baseMiddle2 = (base >> 24)  & 0xff;
    tssEntry->baseHigh = (base >> 32) & 0xffffffff;
    tssEntry->reserved = 0;
}  

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

void initIDT()
{
    idtPtr.limit = sizeof(struct IDTEntry) * 256 - 1;
    idtPtr.base = (u64int) &idtTable;

    memset(idtTable, 0, sizeof(struct IDTEntry)*256);

    setIDT(&idtTable[0], (void*)isr0, 0x08, 0x8e);
    setIDT(&idtTable[1], (void*)isr1, 0x08, 0x8e);
    setIDT(&idtTable[2], (void*)isr2, 0x08, 0x8e);
    setIDT(&idtTable[3], (void*)isr3, 0x08, 0x8e);
    setIDT(&idtTable[4], (void*)isr4, 0x08, 0x8e);
    setIDT(&idtTable[5], (void*)isr5, 0x08, 0x8e);
    setIDT(&idtTable[6], (void*)isr6, 0x08, 0x8e);
    setIDT(&idtTable[7], (void*)isr7, 0x08, 0x8e);
    setIDT(&idtTable[8], (void*)isr8, 0x08, 0x8e);
    setIDT(&idtTable[9], (void*)isr9, 0x08, 0x8e);
    setIDT(&idtTable[10], (void*)isr10, 0x08, 0x8e);
    setIDT(&idtTable[11], (void*)isr11, 0x08, 0x8e);
    setIDT(&idtTable[12], (void*)isr12, 0x08, 0x8e);
    setIDT(&idtTable[13], (void*)isr13, 0x08, 0x8e);
    setIDT(&idtTable[14], (void*)isr14, 0x08, 0x8e);
    setIDT(&idtTable[15], (void*)isr15, 0x08, 0x8e);
    setIDT(&idtTable[16], (void*)isr16, 0x08, 0x8e);
    setIDT(&idtTable[17], (void*)isr17, 0x08, 0x8e);
    setIDT(&idtTable[18], (void*)isr18, 0x08, 0x8e);
    setIDT(&idtTable[19], (void*)isr19, 0x08, 0x8e);
    setIDT(&idtTable[20], (void*)isr20, 0x08, 0x8e);
    setIDT(&idtTable[21], (void*)isr21, 0x08, 0x8e);
    setIDT(&idtTable[22], (void*)isr22, 0x08, 0x8e);
    setIDT(&idtTable[23], (void*)isr23, 0x08, 0x8e);
    setIDT(&idtTable[24], (void*)isr24, 0x08, 0x8e);
    setIDT(&idtTable[25], (void*)isr25, 0x08, 0x8e);
    setIDT(&idtTable[26], (void*)isr26, 0x08, 0x8e);
    setIDT(&idtTable[27], (void*)isr27, 0x08, 0x8e);
    setIDT(&idtTable[28], (void*)isr28, 0x08, 0x8e);
    setIDT(&idtTable[29], (void*)isr29, 0x08, 0x8e);
    setIDT(&idtTable[30], (void*)isr30, 0x08, 0x8e);
    setIDT(&idtTable[31], (void*)isr31, 0x08, 0x8e);
    setIDT(&idtTable[128], (void*)isr128, 0x08, 0x8e);

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

    setIDT(&idtTable[32], (void*)irq0, 0x08, 0x8e);
    setIDT(&idtTable[33], (void*)irq1, 0x08, 0x8e);
    setIDT(&idtTable[34], (void*)irq2, 0x08, 0x8e);
    setIDT(&idtTable[35], (void*)irq3, 0x08, 0x8e);
    setIDT(&idtTable[36], (void*)irq4, 0x08, 0x8e);
    setIDT(&idtTable[37], (void*)irq5, 0x08, 0x8e);
    setIDT(&idtTable[38], (void*)irq6, 0x08, 0x8e);
    setIDT(&idtTable[39], (void*)irq7, 0x08, 0x8e);
    setIDT(&idtTable[40], (void*)irq8, 0x08, 0x8e);
    setIDT(&idtTable[41], (void*)irq9, 0x08, 0x8e);
    setIDT(&idtTable[42], (void*)irq10, 0x08, 0x8e);
    setIDT(&idtTable[43], (void*)irq11, 0x08, 0x8e);
    setIDT(&idtTable[44], (void*)irq12, 0x08, 0x8e);
    setIDT(&idtTable[45], (void*)irq13, 0x08, 0x8e);
    setIDT(&idtTable[46], (void*)irq14, 0x08, 0x8e);

    flushIDT(&idtPtr);
    asm volatile("sti");
}

void initTSS()
{
    gdtPtr.limit = sizeof(struct GDTEntry) * 7 - 1;
    gdtPtr.base = (u64int)&gdtTable;

    setGDT(&gdtTable[0],0,0,0,0);
    setGDT(&gdtTable[1],0,0xffffffff,0x9a,0xcf);
    setGDT(&gdtTable[2],0,0xffffffff,0x92,0xcf);
    setGDT(&gdtTable[3],0,0xffffffff,0xfa,0xcf);
    setGDT(&gdtTable[4],0,0xffffffff,0xf2,0xcf);

    memset(&tss0, 0, sizeof(struct TSS));
    tss0.rsp0 = KERNEL_STACK_TOP;
    setTSS(&gdtTable[5],(u64int)&tss0, sizeof(struct TSS)-1);

    //tssPtr.limit = sizeof(struct TSSEntry) - 1;
    //tssPtr.base = (u64int)&tss0;

    flushGDT(&gdtPtr);
    flushTSS();
}

void setKernelStack(u64int rsp)
{
    tss0.rsp0 = rsp;
}

