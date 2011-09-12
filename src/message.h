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
    size_t size;
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

int Post(int pid, void *buffer, size_t size);
int Notify(int pid, void *buffer, size_t size);
int Send(int pid, void *buffer, size_t size);
int Receive(struct MessageHeader *header, void *buffer, size_t size);
#endif /* MESSAGE_H */

