#ifndef OZIPC_H
#define OZIPC_H

#include "sysdef.h"
#include "message.h"

s64int OzPost(s64int pid, void *buffer, u64int size);

s64int OzSend(s64int pid, void *buffer, u64int size);

s64int OzReceive(struct MessageHeader *header, void *buffer, u64int size);

s64int OzSendReceive(struct MessageHeader *header, void *sendBuffer, u64int sendSize, void *recvBuffer, u64int recvSize);

s64int OzReply(s64int pid, void *buffer, u64int size);

#endif // OZIPC_H
