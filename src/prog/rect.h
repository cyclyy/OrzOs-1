#ifndef RECT_H
#define RECT_H

#define RECT_INIT(rect, x, y, w, h) struct Rect rect = {x, y, w, h};

struct Rect {
    int x, y;
    int w, h;
};

void initRect(struct Rect *rect, int x, int y, int w, int h);

struct Rect *copyRect(struct Rect *dst, const struct Rect *src);

struct Rect *unionRect(struct Rect *rect1, struct Rect *rect2);

struct Rect *crossRect(struct Rect *rect1, struct Rect *rect2);

inline int insideRect(const struct Rect *rect, int x, int y);

int isNullRect(const struct Rect *rect);

int isEmptyRect(const struct Rect *rect);

int rectTop(const struct Rect *rect);

int rectLeft(const struct Rect *rect);

int rectRight(const struct Rect *rect);

int rectBottom(const struct Rect *rect);

struct Rect *translateRect(struct Rect *rect, int deltaX, int deltaY);

struct Rect *makeRect(int x, int y, int w, int h);

#endif /* RECT_H */
