#include "message.h"
#include "task.h"
#include "waitqueue.h"

s64int kGetMessage(u64int *pid, u64int *arg1, u64int *arg2,
        u64int *bodyLen, u64int bufLen, void *buf)
{
    return 0;
}

s64int kPostMessage(u64int pid, u64int arg1, u64int arg2,
        u64int bodyLen, void *bodyBuf)
{
    return 0;
}

s64int kSendMessage(u64int pid, u64int arg1, u64int arg2,
        u64int bodyLen, void *bodyBuf)
{
    return 0;
}

