#include "isr.h"
#include "common.h"
#include "screen.h"

static isr_t interrupt_handlers[256] = {0};

void isr_handler(registers_t regs)
{
    if (interrupt_handlers[regs.int_no]) {
        isr_t handler = interrupt_handlers[regs.int_no];
        handler(&regs);
    } else {
        printk("Isr_code:%d, eip: %p\n",regs.int_no,regs.eip);
        PANIC("unhandled exception.");
    }
}

void irq_handler(registers_t regs)
{
   // Send an EOI (end of interrupt) signal to the PICs.
   // If this interrupt involved the slave.
   if (regs.int_no >= 40)
   {
       // Send reset signal to slave.
       outb(0xA0, 0x20);
   }
   // Send reset signal to master. (As well as slave, if necessary).
   outb(0x20, 0x20);

    if (interrupt_handlers[regs.int_no]) {
        isr_t handler = interrupt_handlers[regs.int_no];
        return handler(&regs);
    }

}

void register_interrupt_handler(u32int n, isr_t handler)
{
    interrupt_handlers[n] = handler;
}

