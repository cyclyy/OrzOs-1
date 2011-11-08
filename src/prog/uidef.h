#ifndef UIDEF_H
#define UIDEF_H

#include "sysdef.h"
#include "mice.h"
#include "rect.h"
#include "libc/list.h"
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
#define UI_EVENT_CREATE_WINDOW        0x1001
#define UI_EVENT_DESTROY_WINDOW       0x1002
#define UI_EVENT_MOVE_WINDOW          0x1003
#define UI_EVENT_NEXT_EVENT           0x1004
#define UI_EVENT_DRAW_RECTANGLE       0x1005

// events received by client window
#define UI_EVENT_MICE                 0x2001

// events received by client widgets
#define UI_EVENT_MICE_ENTER           0x3001
#define UI_EVENT_MICE_LEAVE           0x3002

struct DisplayModeInfo
{
    u8int mode;
    u8int color;
    u16int width;
    u16int height;
    u16int cellBits;
    u64int addr;
}__attribute__((packed));

struct Color
{
    int r, g, b;
};

struct LineStyle
{
    struct Color color;
    int lineWidth;
};

struct FillStyle
{
    struct Color color;
};

// format of message need synchronous reply
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

struct OzUIWindowDrawRectangleRequest
{
    int type;
    unsigned long id;
    struct Rect clipRect, rect;
    struct LineStyle lineStyle;
    struct FillStyle fillStyle;
}__attribute__((packed));

struct OzUIWindowDrawRectangleReply
{
    int ret;
}__attribute__((packed));

struct OzUIWindowDrawTextRequest
{
    int type;
    unsigned long id;
    struct Rect clipRect, rect;
    struct LineStyle lineStyle;
    char text[0];
}__attribute__((packed));

struct OzUICharLayout
{
    wchar_t ch;
    struct Rect rect;
}__attribute__((packed));

struct OzUITextLayout 
{
    int n;
    struct OzUICharLayout charLayout[0];
}__attribute__((packed));

struct OzUIWindowDrawTextReply
{
    int ret;
    struct OzUITextLayout layout;
}__attribute__((packed));

// format of received event
struct OzUIMiceEvent
{
    unsigned long type;
    long x, y;
    unsigned long button;
}__attribute__((packed));

struct OzUIMiceEventNotify
{
    int type;
    unsigned long id;
    struct OzUIMiceEvent miceEvent;
}__attribute__((packed));

// format of request with reply later(by above events)
struct OzUINextEventRequest
{
    int type;
}__attribute__((packed));

struct OzUIWindow;
struct OzUIWindowOperation
{
    void (*onMiceEvent)(struct OzUIWindow *window, struct OzUIMiceEvent *event);
};

struct OzUIWindow
{
    unsigned long id;
    int screenX, screenY, width, height;
    struct OzUIWindowOperation *ops;
    struct ListHead widgetList;
    struct ListHead link;
    // private members
    struct OzUIWidget *miceWidget, *focusWidget;
};

struct OzUIWidget;
struct OzUIWidgetOperation
{
    void (*onCreate)(struct OzUIWidget *widget);
    void (*onDestroy)(struct OzUIWidget *widget);
    void (*onMiceEnter)(struct OzUIWidget *widget);
    void (*onMiceLeave)(struct OzUIWidget *widget);
};

struct OzUIWidget
{
    struct OzUIWindow *window;
    int type;
    struct Rect rect;
    struct OzUIWidgetOperation *ops;
    struct ListHead link;
};

#endif // UIDEF_H
