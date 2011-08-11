#include "timer.h"
#include "interrupt.h"
#include "schedule.h"
#include "task.h"

u64int ticks = 0;

void timerHandler(struct RegisterState *rs)
{
    ++currentTask->ticks;
    --currentTask->slices;
    if (!currentTask->slices) {
        schedule();
    }
}

void initTimer()
{
    u16int divisor = 1193180 / HZ;

    // send command byte
    outb(0x43, 0x36);

    u8int l = divisor & 0xff;
    u8int h = (divisor >> 8) & 0xff;
    outb(0x40, l);
    outb(0x40, h);

}

void startTimer()
{
    registerInterruptHandler(IRQ0, timerHandler);
}

