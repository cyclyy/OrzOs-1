#ifndef MUTEX_H
#define MUTEX_H

#include "semaphore.h"

struct Mutex {
    struct Semaphore *sem;
};

struct Mutex *mtxCreate();

void mtxDestroy(struct Mutex *mtx);

s64int mtxLock(struct Mutex *mtx);

s64int mtxUnlock(struct Mutex *mtx);



#endif /* MUTEX_H */
