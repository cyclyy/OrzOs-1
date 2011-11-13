#include "mutex.h"
#include "semaphore.h"
#include "kmm.h"
#include "util.h"

struct Mutex *mtxCreate()
{
    struct Mutex *mtx;
    mtx = (struct Mutex*)kMalloc(sizeof(struct Mutex));
    mtx->sem = semCreate(1);
    return mtx;
}

void mtxDestroy(struct Mutex *mtx)
{
    semDestroy(mtx->sem);
    kFree(mtx);
}

s64int mtxLock(struct Mutex *mtx)
{
    return semWait(mtx->sem);
}

s64int mtxUnlock(struct Mutex *mtx)
{
    return semSignal(mtx->sem);
}

