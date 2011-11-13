#ifndef MICE_H
#define MICE_H

#include "sysdef.h"

#define MICE_BUTTON_LEFT     1
#define MICE_BUTTON_RIGHT    2
#define MICE_BUTTON_MIDDLE   4

#define MICE_X_SIGN          0x10
#define MICE_Y_SIGN          0x20

#define  MICE_EVENT_NULL     0
#define  MICE_EVENT_PRESS    1
#define  MICE_EVENT_RELEASE  2
#define  MICE_EVENT_MOVE     4

struct MiceEvent
{
    u64int type;
    s64int deltaX, deltaY;
    u64int button;
} __attribute__((packed));

#endif
