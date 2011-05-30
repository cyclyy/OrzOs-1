#include "semaphore.h"
#include "kmm.h"
#include "schedule.h"
#include "waitqueue.h"
#include "util.h"

struct Semaphore *semCreate(u64int count)
{
    struct Semaphore *sem;
    sem = (struct Semaphore*)kMalloc(sizeof(struct Semaphore));
    sem->count = count;
    sem->wq = (struct WaitQueue*)kMalloc(sizeof(struct WaitQueue));
    memset(sem->wq, 0, sizeof(struct WaitQueue));
    return sem;
}

void semDestroy(struct Semaphore *sem)
{
    kFree(sem->wq);
    kFree(sem);
}

s64int semWait(struct Semaphore *sem)
{
    while (1) {
        asm("cli");
        if (sem->count) {
            sem->count--;
            break;
        } else {
            sleepOn(sem->wq);
        }
    }
    return 0;
}

s64int semSignal(struct Semaphore *sem)
{
    asm("cli");
    sem->count++;
    if (sem->wq->num) {
        sem->count--;
        wakeUpOne(sem->wq);
    }
    return 0;
}

