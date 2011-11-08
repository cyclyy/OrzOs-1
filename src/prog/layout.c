#include "layout.h"
#include "utf8.h"
#include <cairo.h>
#include <cairo-ft.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <stdlib.h>

static inline int insideRect(struct Rect *rect, double x, double y)
{
    return  !((x < rect->x) || (x > rect->x + rect->w)
            || (y < rect->y) || (y > rect->y + rect->h));
}

static int tryPutChar(cairo_glyph_t *glyph, double *x, double *y, struct Rect *rect, 
        cairo_text_extents_t *ext, cairo_font_extents_t *fext)
{
    int retry = 0;
    while (retry < 2) {
        if (insideRect(rect, *x, *y - fext->ascent)
                && insideRect(rect, *x + ext->x_advance, *y + fext->descent)) {
            glyph->x = *x;
            glyph->y = *y;
            printf("glyph %lx, x:%lf, y:%lf\n", glyph->index, *x, *y);
            *x += ext->x_advance;
            break;
        } else {
            *x = rect->x;
            *y += fext->height;
            retry++;
        }
    }
    return (retry < 2);
}

int layoutTextToGlyph(cairo_t *cr, 
        cairo_glyph_t **glyphs, 
        cairo_text_extents_t **exts,
        const char *text, 
        struct Rect *rect, 
        int originX, 
        int originY, 
        int flags)
{
    wchar_t *ws;
    char s[5];
    int ulen, wlen, i, n, retry, ret, spaceAdvance;
    double x, y;
    FT_Face face;
    cairo_text_extents_t ext;
    cairo_font_extents_t fext;
    cairo_glyph_t glyph;

    face = cairo_ft_scaled_font_lock_face(cairo_get_scaled_font(cr));
    cairo_font_extents(cr, &fext);
    glyph.x = glyph.y = 0;
    ulen = uslen(text);
    ws = (wchar_t*)malloc((ulen+1)*sizeof(wchar_t));
    wlen = ustowcs(ws, text, ulen);
    *glyphs = cairo_glyph_allocate(wlen);
    *exts = (cairo_text_extents_t*)malloc(wlen*sizeof(cairo_text_extents_t));
    cairo_text_extents(cr, " ", &ext);
    spaceAdvance = ext.x_advance;
    n = ret = 0;

    if (flags == 0) {
        x = rect->x;
        y = rect->y + fext.ascent;
    } else {
        x = originX;
        y = originY;
    }
    if (!insideRect(rect, x, y))
        goto out;
    for (i = 0; i < wlen; i++) {
        switch (ws[i]) {
            break;
        case L'\n':
            x = rect->x;
            y += fext.height;
            break;
        case L' ':
        default:
            (*glyphs)[n].index = FT_Get_Char_Index(face, ws[i]);
            if (!(*glyphs)[n].index)
                continue;
            cairo_glyph_extents(cr, &(*glyphs)[n], 1, &(*exts)[n]);
            if (!tryPutChar(&(*glyphs)[n++], &x, &y, rect, &(*exts)[n], &fext)) {
                ret = n - 1;
                goto out;
            }
        }
    }
    ret = n;
out:
    free(ws);
    return ret;
}
// vim: sw=4 sts=4 et tw=100
