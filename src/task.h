#ifndef TASK_H
#define TASK_H

#include "sysdef.h"
#include "program.h"
#include "vmm.h"

#define TASK_STATE_READY    1
#define TASK_STATE_WAIT     2
#define TASK_STATE_DEAD     4

struct Task {
    u64int pid;
    u64int priority;
    u64int state;
    u64int ticks;
    u64int slices;
    s64int exitCode;
    struct VM *vm;
    struct Program *prog;
    u64int rsp0, rsp3, rip3, rip, rsp, rbp;
    struct Task *next, *prev;
    struct Task *rqNext, *rqPrev;
    struct Task *sqNext, *sqPrev;
};

extern struct Task *currentTask;

void initMultitasking();

void rootTask();

s64int kNewTask(const char *path, u64int flags);

void kExitTask(s64int exitCode);

#endif /* TASK_H */
