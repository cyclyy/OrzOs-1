#include "uidef.h"
#include "uiserver.h"
#include "uiproto.h"
#include "syscall.h"
#include <stdlib.h>

s64int parseMessage(char *buf, u64int size)
{
    struct UIRequestHeader *header;
    header = (struct UIRequestHeader*)buf;
    return header->type;
}

int main()
{
    s64int serverId, msgId;
    char *recvBuf, *replyBuf;
    u64int n;
    struct MessageInfo mi;
    struct UIInitRequest *initReq;
    struct UIConsoleWriteRequest *conWriteReq;
    struct UIInitReply *initReply;
    struct UIConsoleWriteReply *conWriteReply;

    n = 1000;
    recvBuf = (char*)malloc(n);
    replyBuf = (char*)malloc(n);
    initReq = (struct UIInitRequest*)recvBuf;
    conWriteReq = (struct UIConsoleWriteRequest*)recvBuf;
    initReply = (struct UIInitReply*)replyBuf;
    conWriteReply = (struct UIConsoleWriteReply*)replyBuf;

    serverId = OzCreateServer(UI_PORT);
    while ((msgId = OzReceive(serverId, n, recvBuf, &mi)) >= 0) {
        switch (parseMessage(recvBuf, mi.srcSize)) {
        case UI_REQUEST_INIT:
            initReply->type = UI_REPLY_INIT;
            initReply->retCode = 0;
            initReply->clientId = 1;
            if (msgId) {
                OzReply(msgId,sizeof(struct UIInitRequest),initReply);
            }
            break;
        case UI_REQUEST_CONSOLE_WRITE:
            conWriteReply->type = UI_REPLY_CONSOLE_WRITE;
            conWriteReply->retCode = 0;
            if (msgId) {
                OzReply(msgId,sizeof(struct UIConsoleWriteReply),conWriteReply);
            }
            break;
        default:
            ;
        }

    }
    return 0;
}

