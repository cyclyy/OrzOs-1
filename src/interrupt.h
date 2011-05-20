#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "sysdef.h"

#define IRQ0 32
#define IRQ1 33
#define IRQ2 34
#define IRQ3 35
#define IRQ4 36
#define IRQ5 37
#define IRQ6 38
#define IRQ7 39
#define IRQ8 40
#define IRQ9 41
#define IRQ10 42
#define IRQ11 43
#define IRQ12 44
#define IRQ13 45
#define IRQ14 46
#define IRQ15 47

struct RegisterState {
    //u64int ds;
    //u64int rdi, rsi, rbp, rsp, rbx, rdx, rcx, rax;
    u64int number, errcode;
    u64int rip, cs, rflags, rsp, ss;
};

typedef void (*IsrHandlerFunc)(struct RegisterState *);

void registerInterruptHandler(u32int n, IsrHandlerFunc handler);

// general isr handler
void isrDispatcher(struct RegisterState *regs);

// general irq handler
void irqDispatcher(struct RegisterState *rs);

void initInterrupt();


#endif

