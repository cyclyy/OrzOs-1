#include "iorequest.h"
#include "kmm.h"
#include "message.h"
#include "handle.h"
#include "task.h"

struct IORequest *createIORequest(struct VNode *vnode, int op, void *buffer, unsigned long size)
{
    struct IORequest *ior;
    ior = (struct IORequest*)kMalloc(sizeof(struct IORequest));
    ior->vnode = vnode;
    ior->op = op;
    ior->buffer = buffer;
    ior->size = size;
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
        break;
    case IO_SYNC_WRITE:
        break;
    case IO_ASYNC_READ:
        i = htFindIndex(ior->task->handleTable, ior->vnode);
        ev.type = 123;
        ev.op = ior->op;
        ev.fd = i;
        ev.ret = ret;
        Notify(ior->task,&ev,sizeof(struct IOEvent));
        destroyIORequest(ior);
        break;
    case IO_ASYNC_WRITE:
        break;
    }
}

void destroyIORequest(struct IORequest *ior)
{
    kFree(ior);
}

// vim: sw=4 sts=4 et tw=100
