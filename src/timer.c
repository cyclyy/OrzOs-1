#include "timer.h"
#include "isr.h"
#include "screen.h"
#include "task.h"

u32int ticks = 0;

void timer_irq(registers_t *regs)
{
    ticks++;

    handle_timer_queue();

    switch_task();
}

void init_timer(u32int freq)
{
    u32int divisor = 1193180 / freq;

    // send command byte
    outb(0x43, 0x36);

    u8int l = divisor & 0xff;
    u8int h = (divisor >> 8) & 0xff;
    outb(0x40, l);
    outb(0x40, h);

    register_interrupt_handler(IRQ0, &timer_irq);
}

