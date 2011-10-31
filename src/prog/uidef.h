#ifndef UIDEF_H
#define UIDEF_H

#include "sysdef.h"

#define UIDBG(...)  fprintf(dbgFile, __VA_ARGS__)

#define UISERVER_PID        2

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

#define UI_EVENT_CREATE_WINDOW        0x1001
#define UI_EVENT_DESTROY_WINDOW       0x1002
#define UI_EVENT_MOVE_WINDOW          0x1003

struct DisplayModeInfo
{
    u8int mode;
    u8int color;
    u16int width;
    u16int height;
    u16int cellBits;
    u64int addr;
}__attribute__((packed));

struct OzUICreateWindowRequest
{
    int type;
    int width, height, flags;
}__attribute__((packed));

struct OzUICreateWindowReply
{
    unsigned long id;
    int screenX, screenY, width, height, flags;
    int clientX, clientY, clientWidth, clientHeight;
}__attribute__((packed));

struct OzUIDestroyWindowRequest
{
    int type;
    unsigned long id;
}__attribute__((packed));

struct OzUIDestroyWindowReply
{
    int type;
    int ret;
}__attribute__((packed));

struct OzUIMoveWindowRequest
{
    int type;
    unsigned long id;
    int x, y;
}__attribute__((packed));

struct OzUIMoveWindowReply
{
    int ret;
}__attribute__((packed));

#endif // UIDEF_H
