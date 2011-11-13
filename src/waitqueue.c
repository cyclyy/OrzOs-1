#include "waitqueue.h"
#include "kmm.h"
#include "util.h"
#include "schedule.h"

int sleepOn(struct WaitQueue *wq)
{
    currentTask->wakeCode = 0;
    currentTask->state = TASK_STATE_WAIT;
    rqRemove(currentTask);
    wqAppend(wq,currentTask);
    schedule();
    return currentTask->wakeCode;
}

void wakeUpEx(struct WaitQueue *wq, struct Task *task, s64int wakeCode)
{
    struct Task *t;

    if (!wq->num || !task)
        return;

    wq->num--;
    t = task;
    if (wq->head == t)
        wq->head = wq->head->sqNext;
    if (wq->tail == t)
        wq->tail = wq->tail->sqPrev;
    t->state = TASK_STATE_READY;
    t->wakeCode = wakeCode;
    rqAdd(t);

    return;
}

void wakeUp(struct WaitQueue *wq, struct Task *task)
{
    wakeUpEx(wq,task,0);
}

void wakeUpOne(struct WaitQueue *wq)
{
    wakeUpEx(wq,wq->head,0);
    /*
    struct Task *t;

    t = wqTakeFirst(wq);

    if (!t)
        return;

    t->state = TASK_STATE_READY;
    rqAdd(t);
    */
}

void wakeUpAll(struct WaitQueue *wq)
{
    while (wq->num)
        wakeUpOne(wq);
}

struct Task *wqTakeFirst(struct WaitQueue *wq)
{
    struct Task *t;

    if (!wq->num)
        return 0;

    wq->num--;
    t = wq->head;
    wq->head = t->sqNext;
    if (wq->head)
        wq->head->sqPrev = 0;
    if (wq->tail == t)
        wq->tail = 0;

    return t;
}

void wqAppend(struct WaitQueue *wq, struct Task *t)
{
    t->sqNext = 0;
    t->sqPrev = wq->tail;
    if (wq->tail)
        wq->tail->sqNext = t;
    wq->tail = t;

    if (!wq->head)
        wq->head = t;

    wq->num++;
}

struct WaitQueue *wqCreate()
{
    struct WaitQueue *wq;

    wq = (struct WaitQueue*)kMalloc(sizeof(struct WaitQueue));
    memset(wq,0,sizeof(struct WaitQueue));

    return wq;
}
