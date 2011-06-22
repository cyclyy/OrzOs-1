#ifndef OZIPC_H
#define OZIPC_H

#include "sysdef.h"

s64int OzCreateServer(s64int port);

s64int OzDestroyServer(s64int serverId);

s64int OzConnect(s64int port);

s64int OzDisconnect(s64int clientId);

s64int OzSend(s64int clientId, char *src, u64int srcSize, char *dest, u64int destSize);

s64int OzPost(s64int clientId, char *src, u64int srcSize);

s64int OzReceive(s64int serverId, char *buf, u64int bufSize);

s64int OzReply(s64int msgId, char *buf, u64int bufSize);

#endif // OZIPC_H
