#include "guiconfig.h"
#include <string.h>
#include <stdlib.h>

#define DEFAULT_BORDER_SIZE     3
#define DEFAULT_TITLE_HEIGHT    24

struct GUIConfig *guiConfig()
{
    static struct GUIConfig *gconfig = 0;
    if (!gconfig) {
        gconfig = (struct GUIConfig*)malloc(sizeof(struct GUIConfig));
        gconfig->borderSize = DEFAULT_BORDER_SIZE;
        gconfig->titleHeight = DEFAULT_TITLE_HEIGHT;
    }
    return gconfig;
}
