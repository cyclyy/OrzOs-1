#include "timer.h"
#include "interrupt.h"
#include "schedule.h"
#include "kmm.h"
#include "task.h"
#include "waitqueue.h"
#include "util.h"
#include "libc/list.h"

struct TimerQueue
{
    struct ListHead list;
    struct WaitQueue *wq;
};


u64int globalTicks = 0;

static struct TimerQueue *tq = 0;

static void timerTick()
{
    struct Timer *t, *tmp;
    listForEachEntrySafe(t, tmp, &tq->list, link) {
        if (t->expireTick <= globalTicks) {
            t->state = TIMER_STOPPED;
            listDel(&t->link);
            t->cb(t->arg);
        } else {
            break;
        }
    }
}

static void timerHandler(struct RegisterState *rs)
{
    ++globalTicks;
    ++currentTask->ticks;
    --currentTask->slices;
    timerTick();
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

void initSoftTimer()
{
    if (!tq) {
        tq = (struct TimerQueue*)kMalloc(sizeof(struct TimerQueue));
        INIT_LIST_HEAD(&tq->list);
        tq->wq = wqCreate();
    }
}

struct Timer *createTimer(long expire, Callback cb, void *arg)
{
    struct Timer *timer;

    timer = (struct Timer*)kMalloc(sizeof(struct Timer));
    INIT_LIST_HEAD(&timer->link);
    timer->expireTick = globalTicks + expire;
    timer->cb = cb;
    timer->arg = arg;
    timer->state = TIMER_STOPPED;

    return timer;
}

int startTimer(struct Timer *timer)
{
    struct Timer *t;
    if ((timer->state != TIMER_STOPPED) && (timer->expireTick <= globalTicks)) {
        return -1;
    }
    timer->state = TIMER_RUNNING;
    listForEachEntry(t, &tq->list, link) {
        if (t->expireTick >= timer->expireTick) {
            listAdd(&timer->link, &t->link);
            return 0;
        }
    }
    listAddTail(&timer->link, &tq->list);
    return 0;
}

int stopTimer(struct Timer *timer)
{
    if (timer->state != TIMER_RUNNING) {
        return -1;
    }
    timer->state = TIMER_STOPPED;
    listDel(&timer->link);
    return 0;
}

void destroyTimer(struct Timer *timer)
{
    if (timer->state == TIMER_RUNNING) {
        stopTimer(timer);
    }
    kFree(timer);
}

