#ifndef TIMER_H
#define TIMER_H

#include "sysdef.h"

struct ExpireNode;
typedef void (*DelayedCallbackFunction)(struct ExpireNode *, void *);

struct ExpireNode
{
    u64int expire;
    DelayedCallbackFunction func;
    void *arg;
    struct ExpireNode *next, *prev;
};

struct TimerQueue
{
    struct ExpireNode *head;
    struct WaitQueue *wq;
};

extern u64int globalTicks;

void initGlobalTimer();

void startGlobalTimer();

struct ExpireNode *addDelayedCallback(u64int expire, DelayedCallbackFunction func, void *arg);

void removeDelayedCallback(struct ExpireNode *enode);

#endif

