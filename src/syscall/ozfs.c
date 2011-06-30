#include "ozfs.h"
#include "sysdef.h"
#include "kmm.h"
#include "vmm.h"
#include "task.h"
#include "util.h"
#include "handle.h"
#include "vfs.h"

s64int doRead(struct VNode *node, u64int size, char *buffer)
{
    char *kbuf;
    s64int ret, len, n;
    kbuf = (char*)kMalloc(PAGE_SIZE);
    ret = 0;
    while (size) {
        len = MIN(size, PAGE_SIZE);
        n = vfsRead(node,len,kbuf);
        if (n<=0) {
            ret = n;
            break;
        }
        n = copyToUser(buffer,kbuf,len);
        if (n<len) {
            ret += n;
            break;
        }
        ret += n;
        size -= n;
    }
    kFree(kbuf);
    return ret;
}

s64int doWrite(struct VNode *node, u64int size, char *buffer)
{
    char *kbuf;
    s64int ret, len, n;
    kbuf = (char*)kMalloc(PAGE_SIZE);
    ret = 0;
    while (size) {
        len = MIN(size, PAGE_SIZE);
        n = copyFromUser(kbuf,buffer,len);
        n = vfsWrite(node,n,kbuf);
        if (n<0) {
            ret = n;
            break;
        } else if (n < len) {
            ret += n;
            break;
        }
        ret += n;
        size -= n;
    }
    kFree(kbuf);
    return ret;
}

s64int doReadDirectory(struct VNode *node, u64int bufSize, char *buf)
{
    char *kbuf;
    s64int ret;
    kbuf = (char*)kMalloc(bufSize);
    ret = vfsReadDirectory(node,bufSize,kbuf);
    if (ret > 0) {
        ret = copyToUser(buf, kbuf, ret);
    }
    kFree(kbuf);
    return ret;
}

s64int OzOpen(char *path, s64int flags)
{
    char s[MAX_NAME_LEN];
    struct VNode *vnode;
    struct Handle *handle;
    s64int i;

    copyFromUser(s,path,MAX_NAME_LEN);
    s[MAX_NAME_LEN-1] = 0;
    vnode = (struct VNode*)kMalloc(sizeof(struct VNode));
    memset(vnode,0,sizeof(struct VNode));
    if (vfsOpen(s,flags,vnode) == 0) {
        i = htFindFreeIndex(currentTask->handleTable);
        if (i<0)
            goto error;
        currentTask->handleTable->used++;
        handle = &currentTask->handleTable->handle[i];
        handle->type = HANDLE_VNODE;
        handle->pointer = vnode;
    }
    return i;
error:
    kFree(vnode);
    return -1;
}

s64int OzClose(s64int fd)
{
    struct Handle *handle;
    s64int ret;

    if (!IS_VALID_HANDLE_INDEX(fd)) {
        return -1;
    }
    handle = &currentTask->handleTable->handle[fd];
    if (handle->type == HANDLE_VNODE) {
        ret = vfsClose((struct VNode*)handle->pointer);
        currentTask->handleTable->used--;
        handle->type = HANDLE_FREE;
        handle->pointer = 0;
        return ret;
    } else {
        return -1;
    }
}

s64int OzRead(s64int fd, u64int size, char *buffer)
{
    struct Handle *handle;

    if (!IS_VALID_HANDLE_INDEX(fd)) {
        return -1;
    }
    
    handle = &currentTask->handleTable->handle[fd];
    if (handle->type == HANDLE_VNODE)  {
        return doRead((struct VNode*)handle->pointer, size, buffer);
    } else {
        return -1;
    }
}

s64int OzWrite(s64int fd, u64int size, char *buffer)
{
    struct Handle *handle;

    if (!IS_VALID_HANDLE_INDEX(fd)) {
        return -1;
    }
    
    handle = &currentTask->handleTable->handle[fd];
    if (handle->type == HANDLE_VNODE) {
        return doWrite((struct VNode*)handle->pointer, size, buffer);
    } else {
        return -1;
    }
}

s64int OzReadDirectory(s64int fd, u64int bufSize, char *buffer)
{
    struct Handle *handle;

    if (!IS_VALID_HANDLE_INDEX(fd)) {
        return -1;
    }
    
    handle = &currentTask->handleTable->handle[fd];
    if (handle->type == HANDLE_VNODE) {
        return doReadDirectory((struct VNode*)handle->pointer, bufSize, buffer);
    } else {
        return -1;
    }
}

s64int OzIoControl(s64int fd, s64int request, u64int size, char *buffer)
{
    struct Handle *handle;

    if (!IS_VALID_HANDLE_INDEX(fd)) {
        return -1;
    }
    
    handle = &currentTask->handleTable->handle[fd];
    if (handle->type == HANDLE_VNODE) {
        return vfsIoControl((struct VNode*)handle->pointer, request,  size, buffer);
    } else {
        return -1;
    }
}

s64int OzSeek(s64int fd, s64int offset, s64int pos)
{
    struct Handle *handle;

    if (!IS_VALID_HANDLE_INDEX(fd)) {
        return -1;
    }
    
    handle = &currentTask->handleTable->handle[fd];
    if (handle->type == HANDLE_VNODE) {
        return vfsSeek((struct VNode*)handle->pointer, offset, pos);
    } else {
        return -1;
    }
}

// vim: sw=4 sts=4 et tw=100
