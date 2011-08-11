#include "interrupt.h"
#include "util.h"
#include "dtable.h"

static IsrHandlerFunc interruptHandlers[256] = {0};

void isrDispatcher(struct RegisterState *rs)
{
    u64int cr2;
    if (interruptHandlers[rs->number]) {
        IsrHandlerFunc handler = interruptHandlers[rs->number];
        handler(rs);
    } else {
        printk("Interrupt %d %x %x\n",rs->number, rs->rip, rs->errcode);
        asm("mov %%cr2, %0":"=r"(cr2));
        printk("cr2 %x\n",cr2);
        //printk("Isr_code:%d, eip: %p\n",rs->number,regs.eip);
        PANIC("unhandled exception.");
    }
}

void irqDispatcher(struct RegisterState *rs)
{
    // Send an EOI (end of interrupt) signal to the PICs.
    // If this interrupt involved the slave.
    if (rs->number >= 40)
    {
        // Send reset signal to slave.
        outb(0xA0, 0x20);
    }
    // Send reset signal to master. (As well as slave, if necessary).
    outb(0x20, 0x20);

    if (interruptHandlers[rs->number]) {
        IsrHandlerFunc handler = interruptHandlers[rs->number];
        return handler(rs);
    }
}

void registerInterruptHandler(u64int n, IsrHandlerFunc handler)
{
    interruptHandlers[n] = handler;
}

void initInterrupt()
{
    initIDT();
    asm ("sti");
}
