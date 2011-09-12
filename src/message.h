#ifndef MESSAGE_H
#define MESSAGE_H

#include "sysdef.h"
#include "semaphore.h"
#include "waitqueue.h"
#include "libc/list.h"

struct MessageHeader
{
    int pid;
    int tstamp;
    unsigned long size;
};

struct Message
{
    int type;
    struct MessageHeader header;
    struct Task *task;
    void *buffer;
    struct ListHead link;
};

struct MessageQueue
{
    struct WaitQueue *wq;
    struct WaitQueue *recvWQ;
    struct ListHead list;
};

struct MessageQueue *mqCreate();

int Post(struct Task *task, void *buffer, unsigned long size);
int Notify(struct Task *task, void *buffer, unsigned long size);
int Send(struct Task *task, void *buffer, unsigned long size);
int Receive(struct MessageHeader *header, void *buffer, unsigned long size);
#endif /* MESSAGE_H */

