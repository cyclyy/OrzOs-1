#ifndef IOREQUEST_H
#define IOREQUEST_H

#include "sysdef.h"
#include "task.h"
#include "vfs.h"
#include "waitqueue.h"
#include "libc/list.h"

#define IO_SYNC_READ       1
#define IO_SYNC_WRITE      2
#define IO_ASYNC_READ      3
#define IO_ASYNC_WRITE     4

struct IORequest
{
    int detached;
    struct VNode *vnode;
    int op;
    void *buffer;
    unsigned long size;
    struct Task *task;
    struct WaitQueue *wq;
    struct ListHead link;
};

struct IORequest *createIORequest(struct VNode *vnode, int op, void *buffer, unsigned long size);
void completeIoRequest(struct IORequest *ior, int ret);
int waitIoResult(struct IORequest *ior);
void cancelIORequest(struct IORequest *ior);
void destroyIORequest(struct IORequest *ior);

#endif /* IOREQUEST_H */
