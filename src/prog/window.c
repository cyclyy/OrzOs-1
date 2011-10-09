#include "window.h"
#include "window_p.h"
#include "gcontext_p.h"
#include "guiconfig.h"
#include <stdlib.h>
#include <string.h>
#include <cairo.h>

LIST_HEAD(windowList);

static void drawWindowBorder(struct Window *window)
{
    int pad;
    cairo_t *cr;
    pad = guiConfig()->borderSize / 2;
    cr = window->pixmap->d->cr;

    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_set_line_width(cr, guiConfig()->borderSize);
    cairo_rectangle(cr, pad, pad, window->width - pad, window->height - pad);
    cairo_move_to(cr, window->clientX - pad, window->clientY - pad);
    cairo_line_to(cr, window->width - pad, window->clientY - pad);
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

struct Window *createWindow(int w, int h, int flags)
{
    struct Window *window;
    struct Pixmap *pixmap;
    struct GC *gc;
    pixmap = createPixmap(w,h);
    window = (struct Window*)malloc(sizeof(struct Window));
    window->pixmap = pixmap;
    window->width = w;
    window->height = h;
    window->screenX = 0;
    window->screenY = 0;
    window->clientX = guiConfig()->borderSize;
    window->clientY = guiConfig()->borderSize*2 + guiConfig()->titleHeight;
    window->clientWidth = w - guiConfig()->borderSize*2;
    window->clientHeight = h - window->clientY - guiConfig()->borderSize;
    INIT_LIST_HEAD(&window->link);

    gc = createGCForWindow(window);
    drawWindowBorder(window);
    drawWindowTitle(window);
    destroyGC(gc);

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
