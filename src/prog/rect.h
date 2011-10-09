#ifndef RECT_H
#define RECT_H

#define RECT_INIT(rect, x, y, w, h) struct Rect rect = {x, y, w, h};

struct Rect {
    int x, y;
    int w, h;
};

void initRect(struct Rect *rect, int x, int y, int w, int h);

struct Rect *unionRect(struct Rect *rect1, struct Rect *rect2);

struct Rect *crossRect(struct Rect *rect1, struct Rect *rect2);

int isNullRect(struct Rect *rect);

#endif /* RECT_H */
