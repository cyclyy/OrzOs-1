#include "window.h"
#include "window_p.h"
#include "gcontext_p.h"
#include "guiconfig.h"
#include "app.h"
#include <stdlib.h>
#include <string.h>
#include <cairo.h>
#include <cairo-ft.h>
#include <ft2build.h>
#include FT_FREETYPE_H

static LIST_HEAD(windowList);

struct ListHead *getWindowList()
{
    return &windowList;
}

static void drawWindowBorder(struct Window *window)
{
    int pad;
    cairo_t *cr;
    pad = guiConfig()->borderSize / 2;
    cr = window->pixmap->d->cr;

    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_set_line_width(cr, guiConfig()->borderSize);
    cairo_rectangle(cr, pad, pad, window->width - pad, window->height - pad);
    cairo_move_to(cr, window->clientRect.x - pad, window->clientRect.y - pad);
    cairo_line_to(cr, window->width - pad, window->clientRect.y - pad);
    cairo_stroke(cr);
}

static void drawWindowTitle(struct Window *window)
{
}

struct Pixmap *createPixmap(int w, int h)
{
    char *buffer;
    struct Pixmap *pixmap;
    buffer = (char*)malloc(w*h*2);
    pixmap = createPixmapForData(w, h, buffer);
    pixmap->ownBuffer = 1;
    return pixmap;
}

struct Pixmap *createPixmapForData(int w, int h, char *buffer)
{
    struct Pixmap *pixmap;
    pixmap = (struct Pixmap*)malloc(sizeof(struct Pixmap));
    pixmap->width = w;
    pixmap->height = h;
    pixmap->format = PF_RGB565;
    memset(buffer, 0, w*h*2);
    pixmap->buffer = buffer;
    pixmap->ownBuffer = 0;
    pixmap->d = (struct PixmapPrivate*)malloc(sizeof(struct PixmapPrivate));
    pixmap->d->surface = cairo_image_surface_create_for_data((unsigned char*)buffer,
            CAIRO_FORMAT_RGB16_565,
            w,
            h,
            w*2);
    pixmap->d->cr = cairo_create(pixmap->d->surface);
    cairo_set_font_face(pixmap->d->cr, guiConfig()->fontFace);
    return pixmap;
}

void destroyPixmap(struct Pixmap *pixmap)
{
    cairo_destroy(pixmap->d->cr);
    cairo_surface_destroy(pixmap->d->surface);
    if (pixmap->ownBuffer)
        free(pixmap->buffer);
    free(pixmap->d);
    free(pixmap);
}

struct Window *createWindow(int pid, int w, int h, int flags)
{
    struct Window *window;
    struct Pixmap *pixmap;
    struct GC *gc;
    struct App *app;

    app = getApp(pid);
    if (!app)
        app = createApp(pid);

    pixmap = createPixmap(w,h);

    window = (struct Window*)malloc(sizeof(struct Window));
    window->app = app;
    window->pixmap = pixmap;
    window->width = w;
    window->height = h;
    window->screenX = 0;
    window->screenY = 0;
    window->clientRect.x = guiConfig()->borderSize;
    window->clientRect.y = guiConfig()->borderSize*2 + guiConfig()->titleHeight;
    window->clientRect.w = w - guiConfig()->borderSize*2;
    window->clientRect.h = h - window->clientRect.y - guiConfig()->borderSize;
    INIT_LIST_HEAD(&window->appLink);
    INIT_LIST_HEAD(&window->link);

    gc = createGCForWindow(window);
    drawWindowBorder(window);
    drawWindowTitle(window);
    destroyGC(gc);

    listAdd(&window->appLink, &app->windowList);
    listAddTail(&window->link, &windowList);

    return window;
}

void destroyWindow(struct Window *window)
{
    listDel(&window->link);
    destroyPixmap(window->pixmap);
    free(window);
}

struct GC *createGCForPixmap(struct Pixmap *pixmap)
{
    struct GC *gc;
    gc = (struct GC*)malloc(sizeof(struct GC));
    gc->d = (struct GCPrivate*)malloc(sizeof(struct GCPrivate));
    gc->d->cr = pixmap->d->cr;
    return gc;
}

struct GC *createGCForWindow(struct Window *window)
{
    return createGCForPixmap(window->pixmap);
}

void destroyGC(struct GC *gc)
{
    free(gc->d);
    free(gc);
}

void paintAllWindows(struct GC *gc)
{
    struct Window *window;
    listForEachEntry(window, &windowList, link) {
        paintWindow(gc, window);
    }
}

void paintWindow(struct GC *gc, struct Window *window)
{
    cairo_t *cr;
    cr = gc->d->cr;
    cairo_set_source_surface(cr, window->pixmap->d->surface, window->screenX, window->screenY);
    cairo_paint(cr);
}

void moveWindow(struct Window *window, int x, int y)
{
    window->screenX = x;
    window->screenY = y;
}

struct Window *findWindowUnder(int x, int y)
{
    struct Window *window ;
    listForEachEntryReverse(window, &windowList, link) {
        if ((x >= window->screenX) && (x <= window->screenX + window->width) 
                && (y >= window->screenY) && (y <= window->screenY + window->height))
            return window;
    }
    return 0;
}

unsigned long windowId(struct Window *window)
{
    return (unsigned long)window;
}

struct Window *getWindowById(unsigned long id)
{
    struct Window *window ;
    listForEachEntry(window, &windowList, link) {
        if ((unsigned long)window == id)
            return window;
    }
    return 0;
}

void unionWindowRect(struct Rect *rect, struct Window *window)
{
    struct Rect r2;
    initRect(&r2, window->screenX, window->screenY, window->width, window->height);
    unionRect(rect, &r2);
}

int drawRectangle(struct Window *window, struct Rect *clipRect,
        struct Rect *rect, struct LineStyle *lineStyle, struct FillStyle *fillStyle)
{
    cairo_t *cr;
    double borderPadding;
    cr = window->pixmap->d->cr;

    cairo_save(cr);
    cairo_translate(cr, window->clientRect.x, window->clientRect.y);
    cairo_rectangle(cr, clipRect->x, clipRect->y, clipRect->w, clipRect->h);
    cairo_clip(cr);
    cairo_set_source_rgb(cr, lineStyle->color.r/255.0, lineStyle->color.g/255.0,
            lineStyle->color.b/255.0);
    cairo_set_line_width(cr, lineStyle->lineWidth);
    borderPadding = lineStyle->lineWidth/2.0;
    cairo_rectangle(cr, rect->x + borderPadding, rect->y + borderPadding, rect->w - borderPadding*2.0, rect->h - borderPadding*2.0);
    cairo_stroke_preserve(cr);
    cairo_set_source_rgb(cr, fillStyle->color.r/255.0, fillStyle->color.g/255.0,
            fillStyle->color.b/255.0);
    cairo_fill(cr);
    cairo_reset_clip(cr);
    cairo_restore(cr);
    return 0;
}

int drawTextLayouted(const wchar_t *text, const struct TextLayout *layout, const struct LayoutConstraint *lc)
{
    int n;
    cairo_t *cr;
    cairo_glyph_t *glyphs;
    struct CharLayout *charLayout;

    n = 0;
    cr = lc->cr;

    glyphs = cairo_glyph_allocate(layout->chars);
    listForEachEntry(charLayout, &layout->charList, link) {
        if (!isEmptyRect(&charLayout->rect)) {
            glyphs[n].index = charLayout->glyphIndex;
            glyphs[n].x = charLayout->rect.x;
            glyphs[n].y = charLayout->rect.y + layout->ascent;
            n++;
        }
    }
    cairo_show_glyphs(cr, glyphs, n);

    return layout->chars;
}


static void packOzUITextLayout(struct OzUITextLayout *utl, const struct TextLayout *layout)
{
    int i;
    struct CharLayout *cl;

    utl->chars = layout->chars;
    copyRect(&utl->rect, &layout->rect);
    utl->ascent = layout->ascent;
    utl->descent = layout->descent;
    utl->height = layout->height;
    i = 0;
    listForEachEntry(cl, &layout->charList, link) {
        utl->charLayout[i].glyphIndex = cl->glyphIndex;
        copyRect(&utl->charLayout[i].rect, &cl->rect);
        i++;
    }
}

int drawText(struct Window *window, struct Rect *clipRect,
        struct OzUITextLayoutConstraint *tlc, wchar_t *text, 
        struct LineStyle *lineStyle, struct OzUITextLayout *utl)
{
    int ret;
    cairo_t *cr;
    struct LayoutConstraint lc;
    struct TextLayout *layout;

    cr = window->pixmap->d->cr;

    cairo_save(cr);
    cairo_translate(cr, window->clientRect.x, window->clientRect.y);
    cairo_rectangle(cr, clipRect->x, clipRect->y, clipRect->w, clipRect->h);
    cairo_clip(cr);
    cairo_set_source_rgb(cr, lineStyle->color.r/255.0, lineStyle->color.g/255.0,
            lineStyle->color.b/255.0);
    cairo_set_line_width(cr, lineStyle->lineWidth);
    cairo_set_font_size(cr, tlc->fontSize);

    lc.cr = cr;
    copyRect(&lc.rect, &tlc->rect);
    lc.originX = tlc->originX;
    lc.originY = tlc->originY;
    lc.flags = tlc->flags;

    layout = createTextLayout();
    layoutText(layout, text, &lc);

    packOzUITextLayout(utl, layout);

    ret = drawTextLayouted(text, layout, &lc);

    destroyTextLayout(layout);

    cairo_reset_clip(cr);
    cairo_restore(cr);
    return ret;
}

