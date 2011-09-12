#include "ozipc.h"
#include "message.h"
#include "task.h"
#include "handle.h"
#include "kmm.h"
#include "vmm.h"

s64int OzPost(s64int pid, void *buffer, u64int size)
{
    return Post(lookupPid(pid), buffer, size);
}

s64int OzSend(s64int pid, void *buffer, u64int size)
{
    return Send(lookupPid(pid), buffer, size);
}

s64int OzReceive(struct MessageHeader *header, void *buffer, u64int size)
{
    struct MessageHeader hdr;
    Receive(&hdr, buffer, size);
    if (header)
        return copyToUser(header, &hdr, sizeof(struct MessageHeader));
    return -1;
}

// vim: sw=4 sts=4 et tw=100
