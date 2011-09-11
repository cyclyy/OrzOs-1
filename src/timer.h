#ifndef TIMER_H
#define TIMER_H

#include "sysdef.h"
#include "libc/list.h"

#define TIMER_STOPPED   0
#define TIMER_RUNNING   1

typedef void (*Callback)(void *);

struct Timer
{
    int state;
    long expireTick;
    Callback cb;
    void *arg;
    struct ListHead link;
};

extern u64int globalTicks;

void initGlobalTimer();

void startGlobalTimer();

void initSoftTimer();

struct Timer *createTimer(long expire, Callback cb, void *arg);

int startTimer(struct Timer *timer);

int stopTimer(struct Timer *timer);

void destroyTimer(struct Timer *timer);

#endif

