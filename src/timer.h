#ifndef TIMER_H
#define TIMER_H

#include "sysdef.h"

#define TIMER_DISABLE   0
#define TIMER_ONESHOT   1
#define TIMER_PERIODIC  2

struct ExpireNode;
typedef void (*DelayedCallbackFunction)(struct ExpireNode *, void *);

struct ExpireNode
{
    int type;
    u64int base;
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

struct ExpireNode *addPeriodicCallback(u64int expire, DelayedCallbackFunction func, void *arg);

struct ExpireNode *addOneshotCallback(u64int expire, DelayedCallbackFunction func, void *arg);

void removeDelayedCallback(struct ExpireNode *enode);

#endif

