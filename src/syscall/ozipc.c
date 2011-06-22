#include "ozipc.h"
#include "message.h"
#include "task.h"
#include "handle.h"
#include "kmm.h"

s64int OzCreateServer(s64int port)
{
    struct Server *server;
    struct Handle *handle;
    u64int i;

    server = kCreateServer(port);
    if (!server) {
        return -1;
    }
    i = htFindFreeIndex(currentTask->handleTable);
    if (i<0) {
        return -1;
    }
    currentTask->handleTable->used++;
    handle = &currentTask->handleTable->handle[i];
    handle->type = HANDLE_SERVER;
    handle->pointer = server;
    return i;
}

s64int OzDestroyServer(s64int serverId)
{
    struct Handle *handle;
    s64int ret;

    if (!IS_VALID_HANDLE_INDEX(serverId)) {
        return -1;
    }
    
    handle = &currentTask->handleTable->handle[serverId];
    if (handle->type == HANDLE_SERVER) {
        ret = kDestroyServer((struct Server*)handle->pointer);
        currentTask->handleTable->used--;
        handle->type = HANDLE_FREE;
        handle->pointer = 0;
        return ret;
    } else {
        return -1;
    }
}

s64int OzConnect(s64int port)
{
    struct Client *client;
    struct Handle *handle;
    u64int i;

    client = kConnect(port);
    if (!client) {
        return -1;
    }
    i = htFindFreeIndex(currentTask->handleTable);
    if (i<0) {
        return -1;
    }
    currentTask->handleTable->used++;
    handle = &currentTask->handleTable->handle[i];
    handle->type = HANDLE_CLIENT;
    handle->pointer = client;

    return i;
}

s64int OzDisconnect(s64int clientId)
{
    struct Handle *handle;
    s64int ret;

    if (!IS_VALID_HANDLE_INDEX(clientId)) {
        return -1;
    }
    
    handle = &currentTask->handleTable->handle[clientId];
    if (handle->type == HANDLE_CLIENT) {
        ret = kDisconnect((struct Client*)handle->pointer);
        currentTask->handleTable->used--;
        handle->type = HANDLE_FREE;
        handle->pointer = 0;
        return ret;
    } else {
        return -1;
    }
}

s64int OzSend(s64int clientId, char *src, u64int srcSize, char *dest, u64int destSize)
{
    struct Handle *handle;

    if (!IS_VALID_HANDLE_INDEX(clientId)) {
        return -1;
    }
    
    handle = &currentTask->handleTable->handle[clientId];
    if (handle->type == HANDLE_CLIENT) {
        return kSend((struct Client*)handle->pointer, src, srcSize, dest, destSize);
    } else {
        return -1;
    }
}

s64int OzPost(s64int clientId, char *src, u64int srcSize)
{
    struct Handle *handle;

    if (!IS_VALID_HANDLE_INDEX(clientId)) {
        return -1;
    }
    
    handle = &currentTask->handleTable->handle[clientId];
    if (handle->type == HANDLE_CLIENT) {
        return kPost((struct Client*)handle->pointer, src, srcSize);
    } else {
        return -1;
    }
}

s64int OzReceive(s64int clientId, char *buf, u64int bufSize)
{
    struct Handle *handle;
    struct Message *msg;
    s64int i;

    if (!IS_VALID_HANDLE_INDEX(clientId)) {
        return -1;
    }
    
    handle = &currentTask->handleTable->handle[clientId];
    if (handle->type == HANDLE_SERVER) {
        msg = (struct Message *)kReceive((struct Server*)handle->pointer, buf, bufSize);
        if (!msg) {
            return 0;
        }
        i = htFindFreeIndex(currentTask->handleTable);
        if (i<0) {
            return -1;
        }
        currentTask->handleTable->used++;
        handle = &currentTask->handleTable->handle[i];
        handle->type = HANDLE_MESSAGE;
        handle->pointer = msg;
        return i;
    } else {
        return -1;
    }
}

s64int OzReply(s64int msgId, char *buf, u64int bufSize)
{
    struct Handle *handle;
    s64int ret;

    if ((!IS_VALID_HANDLE_INDEX(msgId)) || (msgId==0)) {
        return -1;
    }
    
    handle = &currentTask->handleTable->handle[msgId];
    if (handle->type == HANDLE_MESSAGE) {
        ret = kReply((struct Message*)handle->pointer, buf, bufSize);
        currentTask->handleTable->used--;
        handle->type = HANDLE_FREE;
        handle->pointer = 0;
        return ret;
    } else {
        return -1;
    }
}

// vim: sw=4 sts=4 et tw=100
