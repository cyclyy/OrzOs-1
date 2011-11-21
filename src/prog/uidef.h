#ifndef UIDEF_H
#define UIDEF_H

#include "rect.h"
#include "uimice.h"
#include "uigraphic.h"
#include "uitextlayout.h"
#include <os/sysdef.h>
#include <os/list.h>
#include <wchar.h>

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

// events received by uiserver
#define OZUI_EVENT_CREATE_WINDOW        0x1001
#define OZUI_EVENT_DESTROY_WINDOW       0x1002
#define OZUI_EVENT_MOVE_WINDOW          0x1003
#define OZUI_EVENT_NEXT_EVENT           0x1004
#define OZUI_EVENT_DRAW_TEXT            0x1005
#define OZUI_EVENT_DRAW_RECTANGLE       0x1006
#define OZUI_EVENT_DRAW_LINE            0x1007

// events received by client window
#define OZUI_EVENT_MICE                 0x2001

// events received by client widgets
#define OZUI_EVENT_MICE_ENTER           0x3001
#define OZUI_EVENT_MICE_LEAVE           0x3002
#define OZUI_EVENT_MICE_DOWN            0x3003
#define OZUI_EVENT_MICE_UP              0x3004
#define OZUI_EVENT_MICE_MOVE            0x3005

// format of received event
#define OZUI_MICE_EVENT_MOVE  MICE_EVENT_MOVE
#define OZUI_MICE_EVENT_DOWN  MICE_EVENT_PRESS
#define OZUI_MICE_EVENT_UP    MICE_EVENT_RELEASE 

#define OZUI_MICE_BUTTON_LEFT   MICE_BUTTON_LEFT
#define OZUI_MICE_BUTTON_RIGHT  MICE_BUTTON_RIGHT
#define OZUI_MICE_BUTTON_MIDDLE MICE_BUTTON_MIDDLE

struct DisplayModeInfo
{
    u8int mode;
    u8int color;
    u16int width;
    u16int height;
    u16int cellBits;
    u64int addr;
}__attribute__((packed));

struct OzUINextEventRequest
{
    int type;
}__attribute__((packed));

#endif // UIDEF_H
