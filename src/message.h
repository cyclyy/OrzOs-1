#ifndef MESSAGE_H
#define MESSAGE_H

#include "sysdef.h"
#include "semaphore.h"

struct Message {
    u64int flags;
    u64int pid;
    u64int arg1;
    u64int arg2;
    u64int bodyLen;
    void *bodyBuf;
    struct Message *next, *prev;
};

struct MessageQueue {
    u64int num;
    struct  Semaphore *semGetting, *semSending;
    struct Message *head, *tail;
};

s64int kGetMessage(u64int *pid, u64int *arg1, u64int *arg2,
        u64int *bodyLen, u64int bufLen, void *buf);

s64int kPostMessage(u64int pid, u64int arg1, u64int arg2,
        u64int bodyLen, void *bodyBuf);

s64int kSendMessage(u64int pid, u64int arg1, u64int arg2,
        u64int bodyLen, void *bodyBuf);

#endif /* MESSAGE_H */
