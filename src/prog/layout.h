#ifndef LAYOUT_H
#define LAYOUT_H

#include "rect.h"
#include <os/list.h>
#include <cairo.h>
#include <wchar.h>

#define EOL     L'\n'
#define EOT     L'\0'

struct CharLayout
{
    unsigned int glyphIndex;
    struct Rect rect;
    struct ListHead link;
};

struct TextLayout
{
    int chars;
    struct Rect rect;
    int ascent;
    int descent;
    int height;
    struct ListHead charList;
};

#define TEXT_ALIGN_CENTER   1
#define TEXT_ALIGN_VCENTER  2

struct LayoutConstraint
{
    cairo_t *cr;
    struct Rect rect;
    int fontSize;
    int originX, originY;
    int flags;
};

int layoutText(struct TextLayout *layout, const wchar_t *text, const struct LayoutConstraint *constraint);

struct TextLayout *createTextLayout();

void destroyTextLayout(struct TextLayout *layout);

struct CharLayout *createCharLayout(struct TextLayout *layout);

void destroyCharLayout(struct CharLayout *layout);

#endif /* LAYOUT_H */
