#ifndef TASK_H
#define TASK_H

#include "sysdef.h"
#include "vmm.h"

#define TASK_STATE_READY    1
#define TASK_STATE_WAIT     2

struct Task {
    u64int pid;
    u64int priority;
    u64int state;
    u64int ticks;
    u64int slices;
    struct VM *vm;
    u64int rip, rsp, rbp;
    struct Task *next, *prev;
    struct Task *rqNext, *rqPrev;
};

void initMultitasking();

void schedule();

s64int kFork(u64int flags);

#endif /* TASK_H */
