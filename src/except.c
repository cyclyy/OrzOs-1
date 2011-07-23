#include "except.h"
#include "interrupt.h"
#include "util.h"

void commonFaultHandler(struct RegisterState *regs)
{
    PANIC("Interrupt number:%d, errcode:%d",regs->number, regs->errcode);
}

void pageFaultHandler(struct RegisterState *regs)
{
    struct ExceptTable *et;
    u64int cr2;

    et = &exceptTable;
    while (et != &exceptTableEnd) {
        if (et->errorRip == regs->rip) {
            regs->rip = et->fixupRip;
            DBG("found");
            return;
        }
        et++;
    }
    asm("mov %%cr2, %0":"=r"(cr2));
    PANIC("rip:%x, cr2 %x, not handled",regs->rip,cr2);
}

void initCpuExceptions()
{
    registerInterruptHandler(DIVIDE_ZERO_FAULT,commonFaultHandler);
    registerInterruptHandler(GP_FAULT,commonFaultHandler);
    registerInterruptHandler(PAGE_FAULT,pageFaultHandler);
}

// vim: sw=4 sts=4 et tw=100
