#include "timer.h"
#include "interrupt.h"
#include "schedule.h"
#include "kmm.h"
#include "task.h"
#include "waitqueue.h"
#include "util.h"

u64int globalTicks = 0;

struct TimerQueue *globalTimerQueue()
{
    static struct TimerQueue *tq = 0;
    if (!tq) {
        tq = (struct TimerQueue*)kMalloc(sizeof(struct TimerQueue));
        tq->head = 0;
        tq->wq = wqCreate();
    }
    return tq;
};

void reAddPeriodicCallback(struct ExpireNode *p);

void processSoftTimers()
{
    struct TimerQueue *tq;
    struct ExpireNode *p;
    tq =  globalTimerQueue();
    if (tq->head) {
        --tq->head->expire;
        while (tq->head && (tq->head->expire <= 0)) {
            p = tq->head;
            tq->head = p->next;
            if (p->next) {
                p->next->prev = 0;
            }
            p->next = p->prev = 0;
            p->func(p, p->arg);
            switch (p->type) {
            case TIMER_ONESHOT:
                break;
            case TIMER_PERIODIC:
                //reAddPeriodicCallback(p);
                break;
            }
        }
    }
}

void timerHandler(struct RegisterState *rs)
{
    ++globalTicks;
    ++currentTask->ticks;
    --currentTask->slices;
    processSoftTimers();
    if (!currentTask->slices) {
        schedule();
    }
}

void initGlobalTimer()
{
    u16int divisor = 1193180 / HZ;

    // send command byte
    outb(0x43, 0x36);

    u8int l = divisor & 0xff;
    u8int h = (divisor >> 8) & 0xff;
    outb(0x40, l);
    outb(0x40, h);

}

void startGlobalTimer()
{
    registerInterruptHandler(IRQ0, timerHandler);
}

struct ExpireNode *addPeriodicCallback(u64int expire, DelayedCallbackFunction func, void *arg)
{

    struct ExpireNode *enode, *p, *q;
    struct TimerQueue *tq;

    enode = (struct ExpireNode *)kMalloc(sizeof(struct ExpireNode));
    memset(enode,0,sizeof(struct ExpireNode));
    enode->type = TIMER_PERIODIC;
    enode->base = expire;
    enode->expire = expire;
    enode->func = func;
    enode->arg = arg;
    tq = globalTimerQueue();
    q = 0;
    p = tq->head;
    while (p && (expire >= p->expire)) {
        expire -= p->expire;
        q = p;
        p = p->next;
    }
    enode->expire = expire;
    enode->prev = q;
    enode->next = p;
    if (!q) {
        tq->head = enode;
    } else {
        q->next = enode;
        enode->prev = q;
    }
    if (p) {
        p->expire -= expire;
        p->prev = enode;
    }
    return enode;
}

struct ExpireNode *addOneshotCallback(u64int expire, DelayedCallbackFunction func, void *arg)
{
    struct ExpireNode *enode, *p, *q;
    struct TimerQueue *tq;

    enode = (struct ExpireNode *)kMalloc(sizeof(struct ExpireNode));
    memset(enode,0,sizeof(struct ExpireNode));
    enode->type = TIMER_ONESHOT;
    enode->base = expire;
    enode->expire = expire;
    enode->func = func;
    enode->arg = arg;
    tq = globalTimerQueue();
    q = 0;
    p = tq->head;
    while (p && (expire >= p->expire)) {
        expire -= p->expire;
        q = p;
        p = p->next;
    }
    enode->expire = expire;
    enode->prev = q;
    enode->next = p;
    if (!q) {
        tq->head = enode;
    } else {
        q->next = enode;
        enode->prev = q;
    }
    if (p) {
        p->expire -= expire;
        p->prev = enode;
    }
    return enode;
}

void removeDelayedCallback(struct ExpireNode *enode)
{
    struct TimerQueue *tq;
    tq = globalTimerQueue();
    if (enode->expire) {
        if (enode->prev) {
            enode->prev->next = enode->next;
        }
        if (enode->next) {
            enode->next->prev = enode->prev;
        }
        if (tq->head == enode) {
            tq->head = enode->next;
        }
    }
    kFree(enode);
}

