#include "waitqueue.h"
#include "schedule.h"

void sleepOn(struct WaitQueue *wq)
{
    currentTask->state = TASK_STATE_WAIT;
    rqRemove(currentTask);
    currentTask->sqNext = 0;
    currentTask->sqPrev = wq->tail;
    if (wq->tail)
        wq->tail->sqNext = currentTask;
    wq->tail = currentTask;

    if (!wq->head)
        wq->head = currentTask;

    wq->num++;
    schedule();
}

void wakeUpOne(struct WaitQueue *wq)
{
    struct Task *t;

    if (!wq->num)
        return;

    wq->num--;
    t = wq->head;
    t->state = TASK_STATE_READY;
    rqAdd(t);
    wq->head = t->sqNext;
    if (wq->head)
        wq->head->sqPrev = 0;
    if (wq->tail == t)
        wq->tail = 0;
}

void wakeUpAll(struct WaitQueue *wq)
{
    while (wq->num)
        wakeUpOne(wq);
}

