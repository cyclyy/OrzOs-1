#include "oztime.h"
#include "timer.h"
#include "event.h"
#include "message.h"
#include "task.h"
#include "waitqueue.h"

static void alarmCallback(void *arg)
{
    struct TimerEvent ev;
    ev.type = EVENT_TIMER;
    Notify((struct Task*)arg, &ev, sizeof(struct TimerEvent));
}

static void sleepCallback(void *arg)
{
    wakeUpAll((struct WaitQueue*)arg);
}

unsigned long OzMilliAlarm(unsigned long msec)
{
    if (currentTask->timer) {
        destroyTimer(currentTask->timer);
    }
    currentTask->timer = createTimer(msec, alarmCallback, currentTask);
    startTimer(currentTask->timer);
    return 0;
}

unsigned long OzMilliSleep(unsigned long msec)
{
    struct WaitQueue *wq;
    if (currentTask->timer) {
        destroyTimer(currentTask->timer);
    }
    wq = wqCreate();
    currentTask->timer = createTimer(msec, &sleepCallback, wq);
    startTimer(currentTask->timer);
    sleepOn(wq);
    return 0;
}

unsigned long OzGetTicks()
{
    return globalTicks;
}

