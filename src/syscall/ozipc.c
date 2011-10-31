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
    s64int ret;
    ret = Receive(&hdr, buffer, size);
    if (header)
        return copyToUser(header, &hdr, sizeof(struct MessageHeader));
    else
        ret = -1;
    return ret;
}

s64int OzSendReceive(struct MessageHeader *header, void *sendBuffer, u64int sendSize, void *recvBuffer, u64int recvSize)
{
    struct MessageHeader hdr;
    s64int ret;
    if (header) {
        copyFromUser(&hdr, header, sizeof(struct MessageHeader));
        SendReceive(&hdr, sendBuffer, sendSize, recvBuffer, recvSize);
        ret = copyToUser(header, &hdr, sizeof(struct MessageHeader));
    } else
        ret = -1;
    return ret;
}

s64int OzReply(s64int pid, void *buffer, u64int size)
{
    return Reply(lookupPid(pid), buffer, size);
}

// vim: sw=4 sts=4 et tw=100
