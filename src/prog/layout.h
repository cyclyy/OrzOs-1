#ifndef LAYOUT_H
#define LAYOUT_H

#include <cairo.h>

struct Rect {
    int x, y, w, h;
};

int layoutTextToGlyph(cairo_t *cr, 
        cairo_glyph_t **glyphs, 
        cairo_text_extents_t **exts,
        const char *text, 
        struct Rect *rect, 
        int originX, 
        int originY, 
        int flags);

#endif /* LAYOUT_H */
