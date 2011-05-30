#ifndef WAITQUEUE_H
#define WAITQUEUE_H

#include "task.h"

struct WaitQueue {
    u64int num;
    struct Task *head, *tail;
};

void sleepOn(struct WaitQueue *wq);

void wakeUpOne(struct WaitQueue *wq);

void wakeUpAll(struct WaitQueue *wq);

#endif /* WAITQUEUE_H */
