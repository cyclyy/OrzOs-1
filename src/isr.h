#ifndef ISR_H
#define ISR_H

#include "common.h"

typedef struct {
    u32int ds;
    u32int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    u32int int_no, errcode;
    u32int eip, cs, eflags, user_esp, ss;
} registers_t;

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

typedef void (*isr_t)(registers_t *);

void register_interrupt_handler(u32int n, isr_t handler);

// general isr handler
void isr_handler(registers_t regs);

// general irq handler
void irq_handler(registers_t regs);


#endif

