#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "sysdef.h"
#include "waitqueue.h"

struct Semaphore {
    u64int count;
    struct WaitQueue *wq;
};

struct Semaphore *semCreate(u64int count);

void semDestroy(struct Semaphore *sem);

s64int semWait(struct Semaphore *sem);

s64int semSignal(struct Semaphore *sem);

#endif /* SEMAPHORE_H */
