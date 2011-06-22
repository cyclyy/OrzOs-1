#ifndef WAITQUEUE_H
#define WAITQUEUE_H

#include "task.h"

struct WaitQueue {
    u64int num;
    struct Task *head, *tail;
};

void sleepOn(struct WaitQueue *wq);

void wakeUpEx(struct WaitQueue *wq, struct Task *task, s64int wakeCode);

void wakeUp(struct WaitQueue *wq, struct Task *task);

void wakeUpOne(struct WaitQueue *wq);

void wakeUpAll(struct WaitQueue *wq);

struct Task *wqTakeFirst(struct WaitQueue *wq);

void wqAppend(struct WaitQueue *wq, struct Task *t);

struct WaitQueue *wqCreate();

#endif /* WAITQUEUE_H */
