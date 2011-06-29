#include "handle.h"
#include "kmm.h"
#include "util.h"
#include "vfs.h"
#include "message.h"

s64int htFindFreeIndex(struct HandleTable *ht)
{
    s64int i;

    for (i=0; i<MAX_HANDLE_NUMBER; i++) {
        if (ht->handle[i].type == HANDLE_FREE)
            return i;
    }

    return -1;
}

struct HandleTable *htCreate()
{
    struct HandleTable *ht;

    ht = (struct HandleTable *)kMalloc(sizeof(struct HandleTable));
    memset(ht,0,sizeof(struct HandleTable));
    ht->ref = 1;

    return ht;
}

void hClose(struct Handle *handle)
{
    if (!handle) {
        return;
    }
    switch (handle->type) {
    case HANDLE_FREE:
        break;
    case HANDLE_SERVER:
        kDestroyServer((struct Server*)handle->pointer);
        break;
    case HANDLE_CLIENT:
        kDisconnect((struct Client*)handle->pointer);
        break;
    case HANDLE_VNODE:
        vfsClose((struct VNode*)handle->pointer);
        break;
    }
}

void htDestory(struct HandleTable *ht)
{
    s64int i;

    if (!ht) {
        return;
    }
    for (i=1; i<MAX_HANDLE_NUMBER; i++) {
        hClose(&ht->handle[i]);
    }
    kFree(ht);
}

struct HandleTable *htRef(struct HandleTable *ht)
{
    ht->ref++;
    return ht;
}

s64int htDeref(struct HandleTable *ht)
{
    if (ht->ref) {
        ht->ref--;
        if (ht->ref == 0) {
            htDestory(ht);
        }
        return 0;
    } else {
        return -1;
    }
}

// vim: sw=4 sts=4 et tw=100
