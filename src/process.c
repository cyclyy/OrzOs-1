/*
 * =============================================================================
 *
 *       Filename:  process.c
 *
 *    Description:  Multitasking
 *
 *        Version:  1.0
 *        Created:  2011年05月18日 10时16分44秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Wang Hoi (whoi), fearee@gmail.com
 *        Company:  
 *
 * =============================================================================
 */

#include "process.h"
#include "kmm.h"
#include "util.h"
#include "bootinfo.h"
#include "interrupt.h"

#define BASE_QUANTUM    10
#define MLFQ_LEVELS     4
#define MLFQ_MAX_LEVEL  (MLFQ_LEVELS-1)

struct Process *runQueue[MLFQ_LEVELS];
struct Process *procQueue;

struct Process *currentProcess;

void moveKernelStack()
{
    u64int offset;
    u64int oldRsp, oldRbp;
    u64int newRsp, newRbp;
    u64int i;
    u64int paddr, addr;

    kMallocEx(KERNEL_STACK_SIZE,1,&paddr);

    if (!paddr)
        printk("FAILED TO ALLOC MEM");

    mapPagesVtoP(KERNEL_STACK_TOP-KERNEL_STACK_SIZE, 
            paddr, 
            KERNEL_STACK_SIZE/PAGE_SIZE, 
            getPML4E());

    offset = KERNEL_STACK_TOP - getBootInfo()->initialStack;

    asm("mov %%rsp, %0" : "=r"(oldRsp));
    asm("mov %%rbp, %0" : "=r"(oldRbp));

    newRsp = oldRsp + offset;
    newRbp = oldRbp + offset;

    memcpy((void*)newRsp, (void*)oldRsp, getBootInfo()->initialStack - oldRsp);

    // fool walk the stack to correct rbp ;
    for (i=newRsp; i<KERNEL_STACK_TOP; i+=PTR_SIZE) {
        addr = *(u64int *)i;
        if ((oldRsp<=addr) && (addr<getBootInfo()->initialStack)) {
            *(u64int *)i = addr + offset;
        }
    }

    asm("mov %0, %%rsp" :: "r"(newRsp));
    asm("mov %0, %%rbp" :: "r"(newRbp));
}

void timerHandler(struct RegisterState *rs)
{
    ++currentProcess->ticks;
    --currentProcess->slices;
    if (!currentProcess->slices) {
        schedule();
    }
}

void initMultitasking()
{
    struct Process *proc;

    moveKernelStack();

    proc = (struct Process *)kMalloc(sizeof(struct Process));
    memset(proc, 0, sizeof(struct Process));
    proc->slices = BASE_QUANTUM;
    proc->vm = vmCreate();

    currentProcess = proc;
    memset(runQueue,0,PTR_SIZE*MLFQ_LEVELS);
    runQueue[0] = currentProcess;
    currentProcess->rqNext = currentProcess->rqPrev = currentProcess;

    procQueue = currentProcess;

    registerInterruptHandler(IRQ0, timerHandler);
}

void schedule()
{
    struct Process *newProcess;
    u64int cprio,nprio,i;

    cprio = currentProcess->priority;

    // give up cpu
    if (currentProcess->slices) {
        runQueue[cprio] = currentProcess->rqNext;
    } else {
        if (runQueue[cprio]->rqNext == currentProcess) 
            runQueue[cprio] = 0;
        else
            runQueue[cprio] = runQueue[cprio]->next;

        currentProcess->rqNext->rqPrev = currentProcess->rqPrev;
        currentProcess->rqPrev->rqNext = currentProcess->rqNext;

        // move down
        if (cprio < MLFQ_MAX_LEVEL) {
            nprio = cprio + 1;
            currentProcess->priority++;
        } else 
            nprio = MLFQ_MAX_LEVEL;

        // insert
        if (!runQueue[nprio]) {
            runQueue[nprio] = currentProcess;
            currentProcess->rqNext = currentProcess->rqPrev = currentProcess;
        } else {
            currentProcess->rqNext = runQueue[nprio];
            currentProcess->rqPrev = runQueue[nprio]->rqPrev;
            runQueue[nprio]->rqPrev->rqNext = currentProcess;
            runQueue[nprio]->rqPrev = currentProcess;
        }

        currentProcess->slices = BASE_QUANTUM << nprio;
    }

    // pick up a process to run
    for (i=0; i<MLFQ_LEVELS; i++)
        if (runQueue[i]) {
            newProcess = runQueue[0];
            break;
        }

    if (newProcess == currentProcess)
        return;
}

