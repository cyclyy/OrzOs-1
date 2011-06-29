#include "schedule.h"
#include "task.h"
#include "dtable.h"
#include "util.h"
#include "vmm.h"

#define BASE_QUANTUM    10
#define MLFQ_LEVELS     4
#define MLFQ_MAX_LEVEL  (MLFQ_LEVELS-1)

extern u64int readRIP();

struct Task *runQueue[MLFQ_LEVELS] = {0};

void schedule()
{
    struct Task *newTask;
    u64int cprio,nprio,i;
    u64int rsp, rbp ,rip;

    asm("cli");
    cprio = currentTask->priority;

    if (currentTask->state == TASK_STATE_READY) {
        // give up cpu
        if (currentTask->slices) {
            runQueue[cprio] = currentTask->rqNext;
        } else {
            rqRemove(currentTask);

            // move down
            if (cprio < MLFQ_MAX_LEVEL) {
                nprio = cprio + 1;
            } else 
                nprio = MLFQ_MAX_LEVEL;

            currentTask->priority = nprio;
            rqAdd(currentTask);
        }
    }

    // pick up a process to run
    for (i=0; i<MLFQ_LEVELS; i++)
        if (runQueue[i]) {
            newTask = runQueue[i];
            break;
        }

    if (newTask == 0)
        PANIC("miaowu");

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
    setKernelStack(newTask->rsp0);

    currentTask = newTask;
    //DBG("%d",newTask->pid);
    asm("cli;               \
            mov %0, %%rsp;     \
            mov %1, %%rbp;     \
            mov %2, %%rbx;     \
            mov %3, %%cr3;     \
            movq $0x123, %%rax;\
            sti;               \
            jmp *%%rbx"
            ::"a"(rsp), "c"(rbp), "b"(rip), "d"(currentTask->vm->cr3));

}

void rqAdd(struct Task *task)
{
    struct Task *t;
    u64int prio;

    prio = task->priority;
    t = runQueue[prio];
    while (t && (t->rqNext != runQueue[prio]))
        t = t->rqNext;
    if (!t) {
        task->rqNext = task->rqPrev = task;
        runQueue[prio] = task;
    } else {
        task->rqNext = runQueue[prio];
        task->rqPrev = t;
        runQueue[prio]->rqPrev = task;
        t->rqNext = task;
    }
    task->slices = BASE_QUANTUM << prio;
}

void rqRemove(struct Task *task)
{
    u64int prio;

    prio = task->priority;
    if (runQueue[prio] == task) {
        if (runQueue[prio]->rqNext == task) 
            runQueue[prio] = 0;
        else
            runQueue[prio] = runQueue[prio]->rqNext;
    }


    task->rqNext->rqPrev = task->rqPrev;
    task->rqPrev->rqNext = task->rqNext;
    task->rqNext = task->rqPrev = 0;
    task->slices = 0;
}

