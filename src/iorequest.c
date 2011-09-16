#include "iorequest.h"
#include "kmm.h"
#include "message.h"
#include "handle.h"
#include "event.h"
#include "task.h"

struct IORequest *createIORequest(struct VNode *vnode, int op, void *buffer, unsigned long size)
{
    struct IORequest *ior;
    ior = (struct IORequest*)kMalloc(sizeof(struct IORequest));
    ior->detached = 1;
    ior->vnode = vnode;
    ior->op = op;
    ior->buffer = buffer;
    ior->size = size;
    ior->wq = wqCreate();
    ior->task = currentTask;
    INIT_LIST_HEAD(&ior->link);
    return ior;
}

void completeIoRequest(struct IORequest *ior, int ret)
{
    int i;
    struct IOEvent ev;

    switch (ior->op) {
    case IO_SYNC_READ:
        wakeUpEx(ior->wq, ior->task, ret);
        break;
    case IO_SYNC_WRITE:
        wakeUpEx(ior->wq, ior->task, ret);
        break;
    case IO_ASYNC_READ:
        i = htFindIndex(ior->task->handleTable, ior->vnode);
        ev.type = EVENT_IO_READ;
        ev.op = ior->op;
        ev.fd = i;
        ev.ret = ret;
        Notify(ior->task,&ev,sizeof(struct IOEvent));
        wakeUpEx(ior->wq, ior->task, ret);
        if (ior->detached)
            destroyIORequest(ior);
        break;
    case IO_ASYNC_WRITE:
        i = htFindIndex(ior->task->handleTable, ior->vnode);
        ev.type = EVENT_IO_WRITE;
        ev.op = ior->op;
        ev.fd = i;
        ev.ret = ret;
        Notify(ior->task,&ev,sizeof(struct IOEvent));
        wakeUpEx(ior->wq, ior->task, ret);
        if (ior->detached)
            destroyIORequest(ior);
        break;
    }
}

int waitIoResult(struct IORequest *ior)
{
    ior->detached = 0;
    return sleepOn(ior->wq);
}

void destroyIORequest(struct IORequest *ior)
{
    kFree(ior);
}

// vim: sw=4 sts=4 et tw=100
