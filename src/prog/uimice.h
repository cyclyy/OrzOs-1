#ifndef UIMICE_H
#define UIMICE_H

#include "mice.h"

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


#endif /* UIMICE_H */
