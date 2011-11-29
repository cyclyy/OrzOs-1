#ifndef UITEXTLAYOUT_H
#define UITEXTLAYOUT_H

#include "rect.h"

struct OzUICharLayout
{
    unsigned int glyphIndex;
    struct Rect rect;
}__attribute__((packed));

#define SIZE_OZUI_TEXT_LAYOUT_FOR_TEXT(text) \
    SIZE_OZUI_TEXT_LAYOUT_FOR_CHARS(wcslen(text)+1)
#define SIZE_OZUI_TEXT_LAYOUT_FOR_CHARS(chars) \
    (sizeof(struct OzUITextLayout) \
    + sizeof(struct OzUICharLayout) * ((chars)))
#define SIZE_OZUI_TEXT_LAYOUT(x) \
    SIZE_OZUI_TEXT_LAYOUT_FOR_CHARS(((struct OzUITextLayout*)(x))->chars)
struct OzUITextLayout 
{
    int chars;
    struct Rect rect;
    int ascent;
    int descent;
    int height;
    struct OzUICharLayout charLayout[0];
}__attribute__((packed));

// NOTE: these defines must be consistent with layout.h
#define OZUI_TEXT_ALIGN_CENTER      1
#define OZUI_TEXT_ALIGN_VCENTER     2

struct OzUITextLayoutConstraint 
{
    struct Rect rect;
    int fontSize;
    int originX; 
    int originY;
    int flags;
}__attribute__((packed));

void OzUITextLayoutTransform(struct OzUITextLayout *tl, int deltaX, int deltaY);

#endif /* UITEXTLAYOUT_H */
