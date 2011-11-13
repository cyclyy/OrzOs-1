#ifndef GUICONFIG_H
#define GUICONFIG_H

#include <ft2build.h>
#include FT_FREETYPE_H
#include <cairo.h>
#include <cairo-ft.h>

struct GUIConfig {
    int borderSize;
    int titleHeight;
    cairo_font_face_t *fontFace;
};

struct GUIConfig *guiConfig();

#endif /* GUICONFIG_H */
