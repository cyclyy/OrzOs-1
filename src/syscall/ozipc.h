#ifndef OZIPC_H
#define OZIPC_H

#include "sysdef.h"

struct MessageInfo
{
    u64int srcSize;
} __attribute__((packed));

s64int OzCreateServer(s64int port);

s64int OzDestroyServer(s64int serverId);

s64int OzConnect(s64int port);

s64int OzDisconnect(s64int clientId);

s64int OzSend(s64int clientId, u64int srcSize, void *src, u64int destSize, void *dest);

s64int OzPost(s64int clientId, u64int srcSize, void *src);

s64int OzReceive(s64int serverId, u64int bufSize, void *buf, struct MessageInfo *msgInfo);

s64int OzReply(s64int msgId, u64int bufSize, void *buf);

#endif // OZIPC_H
