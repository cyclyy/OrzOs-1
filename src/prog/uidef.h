#ifndef UIDEF_H
#define UIDEF_H

#include "sysdef.h"

#define UIDBG(...)  fprintf(dbgFile, __VA_ARGS__)

#define UI_PORT     256
#define UI_APP_TYPE_CONSOLEN        0
#define UI_APP_TYPE_CURSOR          1
#define UI_APP_TYPE_GRAPHIC         2

#define UI_REQUEST_INIT             1
#define UI_REPLY_INIT               2
#define UI_REQUEST_CONSOLE_WRITE    3
#define UI_REPLY_CONSOLE_WRITE      4

#define MAX_UI_CONSOLE_WRITE_LEN   100

#ifndef MIN
#define MIN(a,b) ( ((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a,b) ( ((a) > (b)) ? (a) : (b))
#endif

#define DISPLAY_MODE_TEXT          1
#define DISPLAY_MODE_VESA          2

#define DISPLAY_IOCTL_GET_CURRENT_MODE_INFO     1
#define DISPLAY_IOCTL_SET_MODE                  2

struct DisplayModeInfo
{
    u8int mode;
    u8int color;
    u16int width;
    u16int height;
    u16int cellBits;
    u64int addr;
}__attribute__((packed));

struct UIRequestHeader
{
    u64int type;
} __attribute__((packed));

struct UIInitRequest
{
    u64int type;
    char name[MAX_NAME_LEN];
    u64int flags;
} __attribute__((packed));

struct UIInitReply
{
    u64int type;
    s64int retCode;
    s64int clientId;
} __attribute__((packed));

struct UIConsoleWriteRequest
{
    u64int type;
    s64int clientId;
    s64int len;
    char buf[MAX_UI_CONSOLE_WRITE_LEN];
} __attribute__((packed));

struct UIConsoleWriteReply
{
    u64int type;
    s64int retCode;
} __attribute__((packed));

#endif // UIDEF_H
