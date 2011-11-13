#include "layout.h"
#include "utf8.h"
#include <cairo.h>
#include <cairo-ft.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <stdlib.h>

/*
static inline int insideRect(struct Rect *rect, double x, double y)
{
    return  !((x < rect->x) || (x > rect->x + rect->w)
            || (y < rect->y) || (y > rect->y + rect->h));
}
*/

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

static void initTextLayout(struct TextLayout *layout)
{
    layout->chars = 0;
    initRect(&layout->rect, 0, 0, 0, 0);
    INIT_LIST_HEAD(&layout->charList);
}

static struct CharLayout *createCharLayout(struct TextLayout *layout)
{
    struct CharLayout *charLayout;
    charLayout = (struct CharLayout*)malloc(sizeof(struct CharLayout));
    initRect(&charLayout->rect, 0, 0, 0, 0);
    INIT_LIST_HEAD(&charLayout->link);
    ++layout->chars;
    listAddTail(&charLayout->link, &layout->charList);
    return charLayout;
}

int layoutText(struct TextLayout *layout, const wchar_t *text, const struct LayoutConstraint *constraint)
{
    FT_Face face;
    cairo_t *cr;
    cairo_font_extents_t fext;
    cairo_text_extents_t ext;
    cairo_glyph_t glyph;
    int i, x, y, len;
    struct CharLayout *charLayout;

    cr = constraint->cr;
    face = cairo_ft_scaled_font_lock_face(cairo_get_scaled_font(cr));
    cairo_font_extents(cr, &fext);

    initTextLayout(layout);
    layout->ascent = fext.ascent;
    layout->descent = fext.descent;
    layout->height = fext.height;
    len = wcslen(text);
    x = constraint->originX;
    y = constraint->originY;
    for (i = 0; i <= len; i++) {
        charLayout = createCharLayout(layout);
        switch (text[i]) {
        case EOL:
            initRect(&charLayout->rect, x, y, 0, fext.height);
            unionRect(&layout->rect, &charLayout->rect);
            x = rectLeft(&constraint->rect);
            y += fext.height;
            break;
        case EOT:
            initRect(&charLayout->rect, x, y, 0, fext.height);
            unionRect(&layout->rect, &charLayout->rect);
            break;
        default:
            glyph.index = FT_Get_Char_Index(face, text[i]);
            cairo_glyph_extents(cr, &glyph, 1, &ext);
            if (x + ext.x_advance > rectRight(&constraint->rect)) {
                x = rectLeft(&constraint->rect);
                y += fext.height;
            }
            charLayout->glyphIndex = glyph.index;
            initRect(&charLayout->rect, x, y, ext.x_advance, fext.height);
            unionRect(&layout->rect, &charLayout->rect);
            x += ext.x_advance;
        }
    }
    return len;
}

struct TextLayout *createTextLayout()
{
    struct TextLayout *layout;
    layout = (struct TextLayout*)malloc(sizeof(struct TextLayout));
    initTextLayout(layout);
    return layout;
}

void destroyTextLayout(struct TextLayout *layout)
{
//TODO: destroy all char layout
    free(layout);
}

// vim: sw=4 sts=4 et tw=100
