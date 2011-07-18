#include "uidef.h"
#include "uiserver.h"
#include "uiproto.h"
#include "syscall.h"
#include <stdlib.h>
#include <string.h>
#include <png.h>

#define WIDTH 800
#define HEIGHT 600

struct Pixel
{
    char r,g,b;
} __attribute__((packed));

struct Pixel *frameBuffer;

s64int parseMessage(char *buf, u64int size)
{
    struct UIRequestHeader *header;
    header = (struct UIRequestHeader*)buf;
    return header->type;
}

s64int openDisplay()
{
    s64int fd;
    struct DisplayModeInfo mi;
    fd = OzOpen("Device:/Display", 0);
    mi.mode = DISPLAY_MODE_VESA;
    mi.width = WIDTH;
    mi.height = HEIGHT;
    mi.cellBits = 24;
    OzIoControl(fd, DISPLAY_IOCTL_SET_MODE, sizeof(struct DisplayModeInfo), &mi);
    frameBuffer = (struct Pixel*)OzMap(fd, 0, 0, 0);
    int i,j,idx;
    for (i=0; i<HEIGHT; i++) {
        for (j=0; j<WIDTH; j++) {
            idx = i*WIDTH + j;
            frameBuffer[idx].r = 0;
            frameBuffer[idx].g = 255;
            frameBuffer[idx].b = 0;
        }
    }
    return fd;
}

int main()
{
    s64int serverId, msgId, display;
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
    display = openDisplay();
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

