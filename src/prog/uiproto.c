#include "uiproto.h"
#include "uidef.h"
#include "sysdef.h"
#include "syscall.h"
#include <string.h>
#include <stdlib.h>

static s64int coid = 0;

s64int uiInitApp(const char *name, u64int flags)
{
    /*
    struct UIInitRequest initRequest;
    struct UIInitReply initReply;

    coid = OzConnect(UI_PORT);
    strncpy(initRequest.name, name, MAX_NAME_LEN);
    initRequest.type = UI_REQUEST_INIT;
    initRequest.name[MAX_NAME_LEN-1] = 0;
    initRequest.flags = flags;
    OzSend(coid, sizeof(struct UIInitRequest), &initRequest, sizeof(struct UIInitReply), &initReply);
    return initReply.clientId;
    */
    return 0;
}

s64int uiQuitApp(s64int id)
{
    return 0;
}

s64int uiConsoleRead(s64int id, u64int size, void *buf)
{
    return 0;
}

s64int uiConsoleWrite(s64int id, u64int size, void *buf)
{
    /*
    struct UIConsoleWriteRequest writeRequest;
    u64int n, total;
    total = 0;
    writeRequest.type = UI_REQUEST_CONSOLE_WRITE;
    while (size) {
        n = MIN(size, MAX_UI_CONSOLE_WRITE_LEN);
        writeRequest.len = n;
        memcpy(writeRequest.buf, (char*)buf + total, n);
        total += n;
        size -= n;
        OzSend(coid, sizeof(struct UIConsoleWriteRequest), &writeRequest, 0, 0);
    }
    */
    return 0;
}

// vim: sw=4 sts=4 et tw=100
