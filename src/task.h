#ifndef TASK_H
#define TASK_H

#include "sysdef.h"

#define TASK_STATE_READY    1
#define TASK_STATE_WAIT     2
#define TASK_STATE_DEAD     4

struct VM;
struct Program;
struct HandleTable;
struct MessageQueue;

struct Task {
    u64int pid;
    u64int priority;
    u64int state;
    u64int ticks;
    u64int slices;
    s64int exitCode;
    s64int wakeCode;
    struct MessageQueue *mq;
    struct VM *vm;
    struct Program *prog;
    struct HandleTable *handleTable;
    u64int rsp0, rsp3, rip3, rip, rsp, rbp, rax, rbx, rcx, rdx;
    struct Task *next, *prev;
    struct Task *rqNext, *rqPrev;
    struct Task *sqNext, *sqPrev;
};

extern struct Task *currentTask;

void initMultitasking();

void rootTask();

s64int kNewTask(const char *path, u64int flags);

void kExitTask(s64int exitCode);

extern struct Task *currentTask;

struct Task *lookupPid(int pid);

int getPid();

#endif /* TASK_H */
