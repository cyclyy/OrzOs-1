#include "layout.h"
#include "utf8.h"
#include <cairo.h>
#include <cairo-ft.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <stdlib.h>

static void initTextLayout(struct TextLayout *layout)
{
    layout->chars = 0;
    initRect(&layout->rect, 0, 0, 0, 0);
    INIT_LIST_HEAD(&layout->charList);
}

struct CharLayout *createCharLayout(struct TextLayout *layout)
{
    struct CharLayout *charLayout;
    charLayout = (struct CharLayout*)malloc(sizeof(struct CharLayout));
    initRect(&charLayout->rect, 0, 0, 0, 0);
    INIT_LIST_HEAD(&charLayout->link);
    ++layout->chars;
    listAddTail(&charLayout->link, &layout->charList);
    return charLayout;
}

void destroyCharLayout(struct CharLayout *charLayout)
{
    free(charLayout);
}

static void centerTextLayout(struct TextLayout *layout, const struct LayoutConstraint *lc)
{
    int deltaX;
    struct Rect lineRect;
    struct CharLayout *lineStartCL, *lineEndCL, *cl;

    lineStartCL = listFirstEntry(&layout->charList, struct CharLayout, link);
    do {
        initRect(&lineRect, 0, 0, 0, 0);
        unionRect(&lineRect, &lineStartCL->rect);
        lineEndCL = lineStartCL;
        while (!isEmptyRect(&lineEndCL->rect)) {
            lineEndCL = listEntry(lineEndCL->link.next, struct CharLayout, link);
            unionRect(&lineRect, &lineEndCL->rect);
        }
        deltaX = (lc->rect.w - lineRect.w) / 2;

        for (cl = lineStartCL; cl != lineEndCL; cl = listEntry(cl->link.next, struct CharLayout, link))
            translateRect(&cl->rect, deltaX, 0);
        translateRect(&lineEndCL->rect, deltaX, 0);

        lineStartCL = listEntry(lineEndCL->link.next, struct CharLayout, link);
    } while (lineEndCL->glyphIndex != EOT);

    deltaX = (lc->rect.w - layout->rect.w) / 2;
    translateRect(&layout->rect, deltaX, 0);
}

static void vcenterTextLayout(struct TextLayout *layout, const struct LayoutConstraint *lc)
{
    int deltaY;
    struct CharLayout *charLayout;

    deltaY = (lc->rect.h - layout->rect.h) / 2;
    translateRect(&layout->rect, 0, deltaY);
    listForEachEntry(charLayout, &layout->charList, link) {
        translateRect(&charLayout->rect, 0, deltaY);
    }
}

int layoutText(struct TextLayout *layout, const wchar_t *text, const struct LayoutConstraint *lc)
{
    FT_Face face;
    cairo_t *cr;
    cairo_font_extents_t fext;
    cairo_text_extents_t ext;
    cairo_glyph_t glyph;
    int i, x, y, len;
    struct CharLayout *charLayout;

    cr = lc->cr;
    cairo_set_font_size(cr, lc->fontSize);
    face = cairo_ft_scaled_font_lock_face(cairo_get_scaled_font(cr));
    cairo_font_extents(cr, &fext);

    initTextLayout(layout);
    layout->ascent = fext.ascent;
    layout->descent = fext.descent;
    layout->height = fext.height;
    len = wcslen(text);
    x = lc->originX;
    y = lc->originY;
    for (i = 0; i <= len; i++) {
        charLayout = createCharLayout(layout);
        switch (text[i]) {
        case EOL:
            charLayout->glyphIndex = EOL;
            initRect(&charLayout->rect, x, y, 0, fext.height);
            unionRect(&layout->rect, &charLayout->rect);
            x = rectLeft(&lc->rect);
            y += fext.height;
            break;
        case EOT:
            charLayout->glyphIndex = EOT;
            initRect(&charLayout->rect, x, y, 0, fext.height);
            unionRect(&layout->rect, &charLayout->rect);
            break;
        default:
            glyph.index = FT_Get_Char_Index(face, text[i]);
            cairo_glyph_extents(cr, &glyph, 1, &ext);
            if (x + ext.x_advance > rectRight(&lc->rect)) {
                x = rectLeft(&lc->rect);
                y += fext.height;
            }
            charLayout->glyphIndex = glyph.index;
            initRect(&charLayout->rect, x, y, ext.x_advance, fext.height);
            unionRect(&layout->rect, &charLayout->rect);
            x += ext.x_advance;
        }
    }
    if (lc->flags & TEXT_ALIGN_CENTER)
        centerTextLayout(layout, lc);
    if (lc->flags & TEXT_ALIGN_VCENTER)
        vcenterTextLayout(layout, lc);
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
