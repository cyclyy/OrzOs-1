#ifndef WINDOW_P_H
#define WINDOW_P_H

#include <cairo.h>

struct PixmapPrivate {
    cairo_t *cr;
    cairo_surface_t *surface;
};

struct WindowPrivate {
};

#endif /* WINDOW_P_H */
