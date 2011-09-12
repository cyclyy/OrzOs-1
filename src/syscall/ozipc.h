#ifndef OZIPC_H
#define OZIPC_H

#include "sysdef.h"
#include "message.h"

s64int OzPost(s64int pid, void *buffer, u64int size);

s64int OzSend(s64int pid, void *buffer, u64int size);

s64int OzReceive(struct MessageHeader *header, void *buffer, u64int size);

#endif // OZIPC_H
