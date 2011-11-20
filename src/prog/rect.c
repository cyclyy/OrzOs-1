#include "rect.h"
#include <os/sysdef.h>

void initRect(struct Rect *rect, int x, int y, int w, int h)
{
    rect->x = x;
    rect->y = y;
    rect->w = w;
    rect->h = h;
}

struct Rect *copyRect(struct Rect *dst, const struct Rect *src)
{
    initRect(dst, src->x, src->y, src->w, src->h);
    return dst;
}

struct Rect *unionRect(struct Rect *rect1, struct Rect *rect2)
{
    int x,y,w,h;
    if (isNullRect(rect1)) {
        rect1->x = rect2->x;
        rect1->y = rect2->y;
        rect1->w = rect2->w;
        rect1->h = rect2->h;
    } else if (!isNullRect(rect2)) {
        x = MIN(rect1->x, rect2->x);
        y = MIN(rect1->y, rect2->y);
        w = MAX(rect1->x + rect1->w, rect2->x + rect2->w) - x;
        h = MAX(rect1->y + rect1->h, rect2->y + rect2->h) - y;
        rect1->x = x;
        rect1->y = y;
        rect1->w = w;
        rect1->h = h;
    }
    return rect1;
}

struct Rect *crossRect(struct Rect *rect1, struct Rect *rect2)
{
    int x,y,rx,ry;
    x = MAX(rect1->x, rect2->x);
    y = MAX(rect1->y, rect2->y);
    rx = MIN(rect1->x + rect1->w, rect2->x + rect2->w);
    ry = MIN(rect1->y + rect1->h, rect2->y + rect2->h);
    if ((x <= rx) && (y <= ry)) {
        rect1->x = x;
        rect1->y = y;
        rect1->w = rx - x;
        rect1->h = ry - y;
    } else {
        rect1->x = 0;
        rect1->y = 0;
        rect1->w = 0;
        rect1->h = 0;
    }
    return rect1;
}

inline int insideRect(const struct Rect *rect, int x, int y)
{
    return ((x >= rect->x) && (x <= rect->x + rect->w)
        && (y >= rect->y) && (y <= rect->y + rect->h));
}

int isNullRect(const struct Rect *rect)
{
    return ((rect->w == 0) && (rect->h == 0));
}

int isEmptyRect(const struct Rect *rect)
{
    return ((rect->w == 0) || (rect->h == 0));
}

int rectTop(const struct Rect *rect)
{
    return rect->y;
}

int rectLeft(const struct Rect *rect)
{
    return rect->x;
}

int rectRight(const struct Rect *rect)
{
    return rect->x + rect->w;
}

int rectBottom(const struct Rect *rect)
{
    return rect->y + rect->h;
}

struct Rect *translateRect(struct Rect *rect, int deltaX, int deltaY)
{
    rect->x += deltaX;
    rect->y += deltaY;
    return rect;
}

