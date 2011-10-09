#ifndef WINDOW_H
#define WINDOW_H

#include "libc/list.h"
#include <cairo.h>

#define PF_RGB565   1

struct PixmapPrivate;
struct WindowPrivate;
struct GCPrivate;

struct Pixmap {
    int width, height;
    int format;
    int ownBuffer;
    char *buffer;
    struct PixmapPrivate *d;
};

struct Window {
    int screenX, screenY, width, height, flags;
    int clientX, clientY, clientWidth, clientHeight;
    struct Pixmap *pixmap;
    struct ListHead link;
    struct WindowPrivate *d;
};

extern struct ListHead windowList;

struct Pixmap *createPixmap(int w, int h);

struct Pixmap *createPixmapForData(int w, int h, char *buffer);

void destroyPixmap(struct Pixmap *pixmap);

struct Window *createWindow(int w, int h, int flags);

void destroyWindow(struct Window *window);

struct GC *createGCForPixmap(struct Pixmap *pixmap);

struct GC *createGCForWindow(struct Window *window);

void destroyGC(struct GC *gc);

void paintAllWindows(struct GC *gc);

void paintWindow(struct GC *gc, struct Window *window);

void moveWindow(struct Window *window, int x, int y);

struct Window *findWindowUnder(int x, int y);

#endif /* WINDOW_H */
