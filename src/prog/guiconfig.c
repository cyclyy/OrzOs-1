#include "guiconfig.h"
#include <string.h>
#include <stdlib.h>

#define DEFAULT_BORDER_SIZE     3
#define DEFAULT_TITLE_HEIGHT    24

struct GUIConfig *guiConfig()
{
    static struct GUIConfig *gconfig = 0;
    FT_Face face;
    FT_Library library;
    if (!gconfig) {
        gconfig = (struct GUIConfig*)malloc(sizeof(struct GUIConfig));
        gconfig->borderSize = DEFAULT_BORDER_SIZE;
        gconfig->titleHeight = DEFAULT_TITLE_HEIGHT;
        FT_Init_FreeType(&library);
        FT_New_Face(library, "C:/zenhei.ttc", 0, &face);
        gconfig->fontFace = cairo_ft_font_face_create_for_ft_face(face,0);
    }
    return gconfig;
}
