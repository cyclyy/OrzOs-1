#ifndef LAYOUT_H
#define LAYOUT_H

#include "rect.h"
#include "libc/list.h"
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

struct LayoutConstraint
{
    cairo_t *cr;
    struct Rect rect;
    int fontSize;
    int originX, originY;
    int flags;
};

int layoutTextToGlyph(cairo_t *cr, 
        cairo_glyph_t **glyphs, 
        cairo_text_extents_t **exts,
        const char *text, 
        struct Rect *rect, 
        int originX, 
        int originY, 
        int flags);

int layoutText(struct TextLayout *layout, const wchar_t *text, const struct LayoutConstraint *constraint);

struct TextLayout *createTextLayout();

void destroyTextLayout(struct TextLayout *layout);

#endif /* LAYOUT_H */
