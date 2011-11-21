#ifndef WINDOW_H
#define WINDOW_H

#include "rect.h"
#include "point.h"
#include "app.h"
#include "uidef.h"
#include "layout.h"
#include <os/list.h>
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
    struct App *app;
    int screenX, screenY, width, height, flags;
    struct Rect clientRect;
    struct Pixmap *pixmap;
    struct ListHead link;
    struct WindowPrivate *d;
};

struct ListHead *getWindowList();

struct Pixmap *createPixmap(int w, int h);

struct Pixmap *createPixmapForData(int w, int h, char *buffer);

void destroyPixmap(struct Pixmap *pixmap);

struct Window *createWindow(int pid, int w, int h, int flags);

void destroyWindow(struct Window *window);

struct GC *createGCForPixmap(struct Pixmap *pixmap);

struct GC *createGCForWindow(struct Window *window);

void destroyGC(struct GC *gc);

void paintAllWindows(struct GC *gc);

void paintWindow(struct GC *gc, struct Window *window);

void moveWindow(struct Window *window, int x, int y);

struct Window *findWindowUnder(int x, int y);

unsigned long windowId(struct Window *window);

struct Window *getWindowById(unsigned long id);

void unionWindowRect(struct Rect *rect, struct Window *window, struct Rect *rect1);

void unionWindow(struct Rect *rect, struct Window *window);

int drawRectangle(struct Window *window, struct Rect *clipRect,
        struct Rect *rect, struct LineStyle *lineStyle, struct FillStyle *fillStyle);

int drawLine(struct Window *window, struct Rect *clipRect,
        struct Point *p1, struct Point *p2, struct LineStyle *lineStyle);

int drawText(struct Window *window, struct Rect *clipRect,
        struct OzUITextLayoutConstraint *tlc, wchar_t *text, struct LineStyle *lineStyle, struct OzUITextLayout *utl);

struct Window *focusWindow();

void setFocusWindow(struct Window *window);

#endif /* WINDOW_H */
