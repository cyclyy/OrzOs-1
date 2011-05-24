#include "task.h"
#include "kmm.h"
#include "util.h"
#include "bootinfo.h"
#include "interrupt.h"

#define BASE_QUANTUM    10
#define MLFQ_LEVELS     4
#define MLFQ_MAX_LEVEL  (MLFQ_LEVELS-1)

extern u64int readRIP();

struct Task *runQueue[MLFQ_LEVELS];
struct Task *taskQueue;
struct Task *currentTask;

u64int lastPid = 0;

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
    ++currentTask->ticks;
    --currentTask->slices;
    if (!currentTask->slices) {
        schedule();
    }
}

void initMultitasking()
{
    struct Task *task;

    moveKernelStack();

    task = (struct Task *)kMalloc(sizeof(struct Task));
    memset(task, 0, sizeof(struct Task));
    task->slices = BASE_QUANTUM;
    task->vm = vmCreate();

    currentTask = task;
    memset(runQueue,0,PTR_SIZE*MLFQ_LEVELS);
    runQueue[0] = currentTask;
    currentTask->rqNext = currentTask->rqPrev = currentTask;

    taskQueue = currentTask;

    registerInterruptHandler(IRQ0, timerHandler);
}

void schedule()
{
    struct Task *newTask;
    u64int cprio,nprio,i;
    u64int rsp, rbp ,rip;

    cprio = currentTask->priority;

    // give up cpu
    if (currentTask->slices) {
        runQueue[cprio] = currentTask->rqNext;
    } else {
        if (runQueue[cprio]->rqNext == currentTask) 
            runQueue[cprio] = 0;
        else
            runQueue[cprio] = runQueue[cprio]->rqNext;

        currentTask->rqNext->rqPrev = currentTask->rqPrev;
        currentTask->rqPrev->rqNext = currentTask->rqNext;

        // move down
        if (cprio < MLFQ_MAX_LEVEL) {
            nprio = cprio + 1;
            currentTask->priority++;
        } else 
            nprio = MLFQ_MAX_LEVEL;

        // insert
        if (!runQueue[nprio]) {
            runQueue[nprio] = currentTask;
            currentTask->rqNext = currentTask->rqPrev = currentTask;
        } else {
            currentTask->rqNext = runQueue[nprio];
            currentTask->rqPrev = runQueue[nprio]->rqPrev;
            runQueue[nprio]->rqPrev->rqNext = currentTask;
            runQueue[nprio]->rqPrev = currentTask;
        }

        currentTask->slices = BASE_QUANTUM << nprio;
    }

    // pick up a process to run
    for (i=0; i<MLFQ_LEVELS; i++)
        if (runQueue[i]) {
            newTask = runQueue[i];
            break;
        }

    if (newTask == currentTask)
        return;

    asm volatile("mov %%rsp, %0" : "=r"(rsp) );
    asm volatile("mov %%rbp, %0" : "=r"(rbp) );
    rip = readRIP();

    if (rip == 0x123) {
        // newTask
        return;
    }
    currentTask->rsp = rsp;
    currentTask->rbp = rbp;
    currentTask->rip = rip;

    rsp = newTask->rsp;
    rbp = newTask->rbp;
    rip = newTask->rip;
    currentTask = newTask;

    asm("cli;               \
         mov %0, %%rsp;     \
         mov %1, %%rbp;     \
         mov %2, %%rbx;     \
         mov %3, %%cr3;     \
         movq $0x123, %%rax;\
         sti;               \
         jmp *%%rbx"
         ::"r"(rsp), "r"(rbp), "r"(rip), "r"(currentTask->vm->cr3));
         
}

s64int kFork(u64int flags)
{
    struct Task *newTask, *oldTask, *t;
    u64int rip, rsp, rbp;

    newTask = (struct Task *)kMalloc(sizeof(struct Task));
    memset(newTask, 0, sizeof(struct Task));
    newTask->state = TASK_STATE_READY;
    newTask->priority = 0;
    newTask->slices = BASE_QUANTUM;
    newTask->pid = ++lastPid;
    newTask->rip = currentTask->rip;
    newTask->rbp = currentTask->rbp;
    newTask->rsp = currentTask->rsp;
    newTask->vm = vmCopy(currentTask->vm, 0);

    if (taskQueue)
        taskQueue->prev = newTask;
    newTask->next = taskQueue;
    taskQueue = newTask;

    t = runQueue[0];
    while (t && (t->rqNext != runQueue[0]))
        t = t->rqNext;
    if (!t) {
        newTask->rqNext = newTask->rqPrev = newTask;
        runQueue[0] = newTask;
    } else {
        newTask->rqNext = runQueue[0];
        newTask->rqPrev = t;
        runQueue[0]->rqPrev = newTask;
        t->rqNext = newTask;
    }

    oldTask = currentTask;
    rip = readRIP();
    // eip is also the execution start address of child task;
    if (currentTask == oldTask)  {
        /*printk("i'm parent %p\n", current_task->pid);*/
        newTask->rip = rip;
        asm volatile("mov %%rsp, %0" : "=r"(rsp) );
        asm volatile("mov %%rbp, %0" : "=r"(rbp) );
        newTask->rsp = rsp;
        newTask->rbp = rbp;

        return newTask->pid;
    } else {
        /*asm volatile("xchg %bx,%bx");*/
        /*printk("i'm child %p\n", current_task->pid);*/
        return 0;
    }

    return 0;
}

