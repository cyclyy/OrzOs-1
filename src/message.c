#include "message.h"
#include "task.h"
#include "util.h"
#include "schedule.h"
#include "kmm.h"
#include "vmm.h"
#include "waitqueue.h"
#include "timer.h"
#include "libc/list.h"

#define MESSAGE_SEND        1
#define MESSAGE_POST        2
#define MESSAGE_SEND_RECV   3

struct MessageQueue *mqCreate()
{
    struct MessageQueue *mq;

    mq = (struct MessageQueue*)kMalloc(sizeof(struct MessageQueue));
    mq->wq = wqCreate();
    mq->recvWQ = wqCreate();
    INIT_LIST_HEAD(&mq->list);

    return mq;
}

int Post(struct Task *task, void *buffer, unsigned long size)
{
    struct Message *msg;
    msg = (struct Message*)kMalloc(sizeof(struct Message));
    msg->type = MESSAGE_POST;
    msg->header.pid = currentTask->pid;
    msg->header.tstamp = globalTicks;
    msg->header.size = size;
    msg->task = 0;
    msg->buffer = (void*)kMalloc(size);
    copyFromUser(msg->buffer, buffer, size);
    INIT_LIST_HEAD(&msg->link);

    if (!task) {
        kFree(msg);
        return -1;
    }
    listAddTail(&msg->link,&task->mq->list); 
    wakeUpOne(task->mq->recvWQ);
    return 0;
}

int Notify(struct Task *task, void *buffer, unsigned long size)
{
    struct Message *msg;
    msg = (struct Message*)kMalloc(sizeof(struct Message));
    msg->type = MESSAGE_POST;
    msg->header.pid = 0;
    msg->header.tstamp = globalTicks;
    msg->header.size = size;
    msg->task = 0;
    msg->buffer = (void*)kMalloc(size);
    //DBG("%p",msg->buffer);
    memcpy(msg->buffer, buffer, size);
    INIT_LIST_HEAD(&msg->link);

    if (!task) {
        kFree(msg);
        return -1;
    }
    listAddTail(&msg->link,&task->mq->list); 
    wakeUpOne(task->mq->recvWQ);
    return 0;
}

int Send(struct Task *task, void *buffer, unsigned long size)
{
    struct Message *msg;
    msg = (struct Message*)kMalloc(sizeof(struct Message));
    msg->type = MESSAGE_SEND;
    msg->header.pid = currentTask->pid;
    msg->header.tstamp = globalTicks;
    msg->header.size = size;
    msg->task = currentTask;
    msg->buffer = buffer;
    INIT_LIST_HEAD(&msg->link);

    if (!task) {
        kFree(msg);
        return -1;
    }
    listAddTail(&msg->link,&task->mq->list); 
    wakeUpOne(task->mq->recvWQ);
    sleepOn(task->mq->wq);
    return 0;
}

int Receive(struct MessageHeader *header, void *buffer, unsigned long size)
{
    struct Message *msg;
    int ret;
    while (listEmpty(&currentTask->mq->list)) {
        sleepOn(currentTask->mq->recvWQ);
    }
    msg = listFirstEntry(&currentTask->mq->list, struct Message, link);
    memcpy(header, &msg->header, sizeof(struct MessageHeader));
    listDel(&msg->link);
    if (msg->type == MESSAGE_POST) {
        ret = copyToUser(buffer, msg->buffer, MIN(size, msg->header.size));
        kFree(msg->buffer);
    } else {
        ret = vmemcpy(currentTask->vm, buffer, msg->task->vm, msg->buffer, MIN(size, msg->header.size));
        wakeUp(currentTask->mq->wq, msg->task);
    }
    kFree(msg);
    return ret;
}


int SendReceive(struct MessageHeader *header, void *sendBuffer, unsigned long sendSize, void *recvBuffer, unsigned long recvSize)
{
    struct Task *task;
    struct Message *msg;

    msg = (struct Message*)kMalloc(sizeof(struct Message));
    msg->type = MESSAGE_SEND_RECV;
    msg->header.pid = currentTask->pid;
    msg->header.tstamp = globalTicks;
    msg->header.size = sendSize;
    msg->task = currentTask;
    msg->buffer = sendBuffer;
    INIT_LIST_HEAD(&msg->link);

    task = lookupPid(header->pid);
    if (!task) {
        kFree(msg);
        return -1;
    }
    listAddTail(&msg->link,&task->mq->list); 

    currentTask->mq->replyMessage = (struct Message*)kMalloc(sizeof(struct Message));
    msg = currentTask->mq->replyMessage;
    msg->type = MESSAGE_SEND_RECV;
    msg->header.pid = currentTask->pid;
    msg->header.tstamp = globalTicks;
    msg->header.size = recvSize;
    msg->task = currentTask;
    msg->buffer = recvBuffer;
    INIT_LIST_HEAD(&msg->link);
    wakeUpOne(task->mq->recvWQ);
    sleepOn(task->mq->replyWQ);
    memcpy(header, &msg->header, sizeof(struct MessageHeader));
    kFree(currentTask->mq->replyMessage);
    currentTask->mq->replyMessage = 0;

    return header->size;
}

int Reply(struct Task *task, void *buffer, unsigned long size)
{
    struct Message *msg;
    int ret;
    msg = task->mq->replyMessage;
    if (!msg || (msg->task != task))
        return -1;
    ret = MIN(size, msg->header.size);
    vmemcpy(msg->task->vm, msg->buffer, currentTask->vm, buffer, ret);
    msg->header.size = ret;
    wakeUp(currentTask->mq->replyWQ, task);
    return ret;
}

