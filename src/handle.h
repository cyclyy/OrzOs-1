#ifndef HANDLE_H
#define HANDLE_H

#include "sysdef.h"

#define IS_VALID_HANDLE_INDEX(i) ((i>=0) && (i<MAX_HANDLE_NUMBER))

#define HANDLE_FREE         0
#define HANDLE_VNODE        1
#define HANDLE_SERVER       2
#define HANDLE_CLIENT       3
#define HANDLE_MESSAGE      4

struct Handle
{
    u64int type;
    void *pointer;
};

struct HandleTable
{
    u64int used;
    u64int ref;
    struct Handle handle[MAX_HANDLE_NUMBER];
};

void hClose(struct Handle *handle);

s64int htFindFreeIndex(struct HandleTable *ht);

struct HandleTable *htCreate();

void htDestory(struct HandleTable *ht);

struct HandleTable *htRef(struct HandleTable *ht);

s64int htDeref(struct HandleTable *ht);

#endif // HANDLE_H
