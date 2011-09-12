#include "message.h"
#include "task.h"
#include "util.h"
#include "schedule.h"
#include "kmm.h"
#include "vmm.h"
#include "waitqueue.h"
#include "timer.h"
#include "libc/list.h"

#define MESSAGE_SEND   1
#define MESSAGE_POST   2

struct MessageQueue *mqCreate()
{
    struct MessageQueue *mq;

    mq = (struct MessageQueue*)kMalloc(sizeof(struct MessageQueue));
    mq->wq = wqCreate();
    mq->recvWQ = wqCreate();
    INIT_LIST_HEAD(&mq->list);

    return mq;
}

int Post(int pid, void *buffer, size_t size)
{
    struct Message *msg;
    struct Task *task;
    msg = (struct Message*)kMalloc(sizeof(struct Message));
    msg->type = MESSAGE_POST;
    msg->header.pid = currentTask->pid;
    msg->header.tstamp = globalTicks;
    msg->header.size = size;
    msg->task = 0;
    msg->buffer = (void*)kMalloc(size);
    copyFromUser(msg->buffer, buffer, size);
    INIT_LIST_HEAD(&msg->link);

    task = lookupPid(pid);
    if (!task) {
        kFree(msg);
        return -1;
    }
    listAddTail(&msg->link,&task->mq->list); 
    wakeUpOne(task->mq->recvWQ);
    return 0;
}

int Notify(int pid, void *buffer, size_t size)
{
    struct Message *msg;
    struct Task *task;
    msg = (struct Message*)kMalloc(sizeof(struct Message));
    msg->type = MESSAGE_POST;
    msg->header.pid = 0;
    msg->header.tstamp = globalTicks;
    msg->header.size = size;
    msg->task = 0;
    msg->buffer = (void*)kMalloc(size);
    memcpy(msg->buffer, buffer, size);
    INIT_LIST_HEAD(&msg->link);

    task = lookupPid(pid);
    if (!task) {
        kFree(msg);
        return -1;
    }
    listAddTail(&msg->link,&task->mq->list); 
    wakeUpOne(task->mq->recvWQ);
    return 0;
}

int Send(int pid, void *buffer, size_t size)
{
    struct Message *msg;
    struct Task *task;
    msg = (struct Message*)kMalloc(sizeof(struct Message));
    msg->type = MESSAGE_POST;
    msg->header.pid = currentTask->pid;
    msg->header.tstamp = globalTicks;
    msg->header.size = size;
    msg->task = currentTask;
    msg->buffer = buffer;
    INIT_LIST_HEAD(&msg->link);

    task = lookupPid(pid);
    if (!task) {
        kFree(msg);
        return -1;
    }
    listAddTail(&msg->link,&task->mq->list); 
    wakeUpOne(task->mq->recvWQ);
    sleepOn(task->mq->wq);
    return 0;
}

int Receive(struct MessageHeader *header, void *buffer, size_t size)
{
    struct Message *msg;
    int ret;
    if (listEmpty(&currentTask->mq->list)) {
        sleepOn(currentTask->mq->recvWQ);
    } else {
        msg = listFirstEntry(&currentTask->mq->list, struct Message, link);
        listDel(&msg->link);
        if (msg->type == MESSAGE_POST) {
            ret = copyToUser(buffer, msg->buffer, MIN(size, msg->header.size));
            kFree(msg->buffer);
        } else {
            ret = vmemcpy(currentTask->vm, buffer, msg->task->vm, msg->buffer, MIN(size, msg->header.size));
            wakeUp(currentTask->mq->wq, msg->task);
        }
        kFree(msg);
    }
    return ret;
}


