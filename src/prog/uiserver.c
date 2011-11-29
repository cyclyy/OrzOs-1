#include "uiwindow.h"
#include "window_p.h"
#include "gcontext_p.h"
#include "uiproto.h"
#include "window.h"
#include "drawoption.h"
#include "uikey.h"
#include "rect.h"
#include "keyhelper.h"
#include <os/syscall.h>
#include <os/mice.h>
#include <os/key.h>
#include <os/event.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <cairo.h>
#include <cairo-ft.h>

#define WIDTH 800
#define HEIGHT 600
#define CURSOR_SIZE 32
#define FB_SIZE (WIDTH*HEIGHT*2)

FILE *dbgFile;

char *fbAddr = 0;
int fbFD = -1;
struct Pixmap *shadowPixmap;
struct GC *rootGC = 0;
cairo_surface_t *bg_surface;
int cursorX = WIDTH / 2, cursorY = HEIGHT / 2;

int miceFD = -1;
cairo_surface_t *cursor_surface;
struct MiceEvent miceEvent;

int kbdFD = -1;
struct KeyEvent keyEvent;

RECT_INIT(screenRect, 0, 0, WIDTH, HEIGHT);
RECT_INIT(dirtyRect, 0, 0, WIDTH, HEIGHT);
RECT_INIT(cursorRect, 0, 0, 0, 0);

void initDisplay()
{
    struct DisplayModeInfo mi;
    char *s = "Device:/Display";
    fbFD = OzOpen(s, 0);
    mi.mode = DISPLAY_MODE_VESA;
    mi.width = WIDTH;
    mi.height = HEIGHT;
    mi.cellBits = 16;
    OzIoControl(fbFD, DISPLAY_IOCTL_SET_MODE, sizeof(struct DisplayModeInfo), &mi);
    fbAddr = (char*)OzMap(fbFD, 0, 0, 0);
    shadowPixmap = createPixmap(WIDTH, HEIGHT);
    rootGC = createGCForPixmap(shadowPixmap);

    bg_surface = cairo_image_surface_create_from_png("C:/wallpaper.png");
    cursor_surface = cairo_image_surface_create_from_png("C:/cursor.png");
}

void initMice()
{
    miceFD = OzOpen("Device:/Mice", 0);
    initRect(&cursorRect, cursorX - CURSOR_SIZE, cursorY - CURSOR_SIZE, CURSOR_SIZE*2, CURSOR_SIZE*2);
}

void paintBackground(struct GC *gc)
{
    cairo_t *cr;
    cr = gc->d->cr;
    cairo_set_source_surface(cr, bg_surface, 0, 0);
    cairo_paint(cr);
}

void paintCursor(struct GC *gc)
{
    cairo_t *cr;
    cr = gc->d->cr;
    cairo_set_source_surface(cr, cursor_surface, cursorX - CURSOR_SIZE, cursorY - CURSOR_SIZE);
    cairo_paint(cr);
} 

inline void updateFrameBuffer()
{
    //memcpy(fbAddr, shadowPixmap->buffer, FB_SIZE);
    register int i, offset;
    crossRect(&dirtyRect, &screenRect);
    offset = WIDTH * dirtyRect.y * 2;
    for (i = 0; i < dirtyRect.h; i++) {
        memcpy(fbAddr + offset + dirtyRect.x * 2, shadowPixmap->buffer + offset + dirtyRect.x * 2,
              dirtyRect.w * 2);
        offset += WIDTH * 2;
    }
}


/*
struct Pixel
{
    unsigned short r:5;
    unsigned short g:6;
    unsigned short b:5;
} __attribute__((packed));

FILE *f;
struct Pixel *frameBuffer;

FT_Library library;
FT_Face face;

s64int openDisplay()
{
    s64int fd;
    struct DisplayModeInfo mi;
    char *s = "Device:/Display";
    char buf[100];
    strcpy(buf,s);
    fd = OzOpen("Device:/Display", 0);
    mi.mode = DISPLAY_MODE_VESA;
    mi.width = WIDTH;
    mi.height = HEIGHT;
    mi.cellBits = 16;
    OzIoControl(fd, DISPLAY_IOCTL_SET_MODE, sizeof(struct DisplayModeInfo), &mi);
    frameBuffer = (struct Pixel*)OzMap(fd, 0, 0, 0);
    return fd;
}

unsigned char image[HEIGHT][WIDTH];
u32int img2[4][4];

void drawBitmap(FT_Bitmap *bitmap, FT_Int x, FT_Int y)
{
    FT_Int  i, j, p, q;
    FT_Int  x_max = x + bitmap->width;
    FT_Int  y_max = y + bitmap->rows;

    for ( i = x, p = 0; i < x_max; i++, p++ ) {
        for ( j = y, q = 0; j < y_max; j++, q++ ) {
            if ( (i < 0) || (j < 0)  || (i >= WIDTH) || (j >= HEIGHT) ) {
                continue;
            }
            image[j][i] = bitmap->buffer[q * bitmap->width + p];
        }
    }
}

void show_image( void )
{
    int  i, j, idx;

    for ( i = 0; i < HEIGHT; i++ ) {
        for ( j = 0; j < WIDTH; j++ ) {
            idx = i*WIDTH + j;
            frameBuffer[idx].r = image[i][j];
            frameBuffer[idx].g = image[i][j];
            frameBuffer[idx].b = image[i][j];


        }
    }
}


void initFontRender() {
    FT_GlyphSlot slot;
    FT_Matrix matrix;
    FT_Vector pen;
    FT_Error error;
    error = FT_Init_FreeType(&library);
    error = FT_New_Face(library, "C:/zenhei.ttc", 0, &face);
    //error = FT_Set_Char_Size(face, 10*64, 0, 100, 0);
    //slot = face->glyph;
    //error = FT_Load_Char(face, 0x56e7, FT_LOAD_RENDER);
    //error = FT_Load_Char(face, msg[0], FT_LOAD_RENDER);
    //drawBitmap(&slot->bitmap, slot->bitmap_left, HEIGHT - slot->bitmap_top - 100);
    //show_image();
}

void initCairo()
{
    unsigned char *buf;
    cairo_surface_t *surface, *png_surface;
    cairo_t *cr;
    cairo_font_face_t *font_face;
    cairo_text_extents_t te;
    int img_width, img_height;

    memset(img2,0,4*4*4);
    //surface = cairo_image_surface_create_for_data(image, CAIRO_FORMAT_RGB8,WIDTH,HEIGHT,WIDTH);
    surface = cairo_image_surface_create_for_data((unsigned char*)frameBuffer, CAIRO_FORMAT_RGB16_565,WIDTH,HEIGHT,WIDTH*2);
    font_face = cairo_ft_font_face_create_for_ft_face(face,0);
    cr = cairo_create(surface);
    cairo_set_font_face(cr,font_face);
    cairo_set_font_size(cr,20);
    cairo_set_source_rgb(cr, 1,1,1);
    png_surface = cairo_image_surface_create_from_png("C:/wallpaper.png");
    cairo_save(cr);
    img_width = cairo_image_surface_get_width(png_surface);
    img_height = cairo_image_surface_get_height(png_surface);
    cairo_scale(cr,WIDTH/(img_width+.0),HEIGHT/(img_height+.0));
    cairo_set_source_surface(cr, png_surface, 0, 0);
    cairo_paint(cr);
    cairo_restore(cr);
    //cairo_paint(cr);
    cairo_move_to(cr,0.0,0.0);
    cairo_line_to(cr,WIDTH,HEIGHT);
    cairo_move_to(cr,WIDTH,0);
    cairo_line_to(cr,0,HEIGHT);
    cairo_set_line_width(cr,1);
    cairo_stroke(cr);
    //cairo_move_to(cr,WIDTH/2,HEIGHT/2);
    cairo_text_extents(cr, "i", &te);
    //cairo_move_to(cr,WIDTH/2 - te.width / 2 - te.x_bearing,HEIGHT/2 - te.height/2 -te.y_bearing);
    cairo_move_to(cr,WIDTH/2,HEIGHT/2);
    cairo_show_text(cr, "i");
    fprintf(f,"我width:%lf,height:%lf,x_bear:%lf,y_bear:%lf,x_adv:%lf,y_adv:%lf\n",te.width,te.height,te.x_bearing,te.y_bearing,te.x_advance,te.y_advance);
    cairo_font_extents_t fex;
    cairo_font_extents(cr, &fex);
    fprintf(f,"ascent:%lf,descent:%lf,height:%lf,max_x_adv:%lf,max_y_adv:%lf\n",fex.ascent,fex.descent,fex.height,fex.max_x_advance,fex.max_y_advance);
    //cairo_rectangle(cr,0,0,1,1);
    //cairo_fill(cr);
    cairo_surface_flush(surface);
    //show_image();
}
*/
void paintAll()
{
    cairo_t *cr;
    if (isNullRect(&dirtyRect))
        return;
    cr = rootGC->d->cr;
    cairo_rectangle(cr, dirtyRect.x, dirtyRect.y, dirtyRect.w, dirtyRect.h);
    cairo_clip(cr);
    paintBackground(rootGC);
    paintAllWindows(rootGC);
    paintCursor(rootGC);
    updateFrameBuffer();
    cairo_reset_clip(rootGC->d->cr);
    initRect(&dirtyRect, 0, 0, 0, 0);
}

int main()
{
    char *buf;
    char replyBuf[3000];
    struct Window *window, *oldWindow;
    struct MessageHeader hdr;
    struct IOEvent *ioEventPtr;
    struct OzUICreateWindowRequest *createWindowRequest;
    struct OzUICreateWindowReply *createWindowReply = (struct OzUICreateWindowReply*)replyBuf;
    struct OzUIDestroyWindowRequest *destroyWindowRequest;
    struct OzUIDestroyWindowReply *destroyWindowReply = (struct OzUIDestroyWindowReply*)replyBuf;
    struct OzUIMoveWindowRequest *moveWindowRequest;
    struct OzUIMoveWindowReply *moveWindowReply = (struct OzUIMoveWindowReply*)replyBuf;
    struct OzUIWindowDrawRectangleRequest *drawRectangleRequest;
    struct OzUIWindowDrawRectangleReply *drawRectangleReply = (struct OzUIWindowDrawRectangleReply*)replyBuf;
    struct OzUIWindowDrawLineRequest *drawLineRequest;
    struct OzUIWindowDrawLineReply *drawLineReply = (struct OzUIWindowDrawLineReply*)replyBuf;
    struct OzUIMiceEventNotify *miceEventNotify = (struct OzUIMiceEventNotify*)replyBuf;
    //struct OzUIFocusEventNotify *focusEventNotify = (struct OzUIFocusEventNotify*)replyBuf;
    struct OzUIKeyEventNotify *keyEventNotify = (struct OzUIKeyEventNotify*)replyBuf;
    //struct OzUINextEventRequest *nextEventRequest = (struct OzUINextEventRequest*)buf;
    struct OzUIWindowDrawTextRequest *drawTextRequest;
    struct OzUIWindowDrawTextReply *drawTextReply = (struct OzUIWindowDrawTextReply*)replyBuf;
    struct OzUIWindowQueryTextLayoutRequest *layoutTextRequest;
    struct OzUIWindowQueryTextLayoutReply *layoutTextReply = (struct OzUIWindowQueryTextLayoutReply*)replyBuf;
    struct OzUIWindowDrawTextLayoutRequest *drawTextLayoutRequest;
    struct OzUIWindowDrawTextLayoutReply *drawTextLayoutReply = (struct OzUIWindowDrawTextLayoutReply*)replyBuf;
    struct App *app;
    //int drawTextReplyLen;

    dbgFile = fopen("Device:/Debug", "w");
    initDisplay();
    initMice();
    kbdFD = OzOpen("Device:/Keyboard", 0);

    OzNewTask("C:/uiclient",0);

    buf = (char*)malloc(10000);

    OzMilliAlarm(40);
    OzReadAsync(kbdFD, sizeof(struct KeyEvent), &keyEvent);
    OzReadAsync(miceFD, sizeof(struct MiceEvent), &miceEvent);
    for (;;) {
        //paintAll();
        OzReceive(&hdr, buf, 10000);
        switch (GET_EVENT_TYPE(buf)) {
        case EVENT_TIMER:
            OzMilliAlarm(40);
            paintAll();
            break;
        case EVENT_IO_READ:
            ioEventPtr = (struct IOEvent*)buf;
            if (ioEventPtr->fd == miceFD) {
                window = findWindowUnder(cursorX, cursorY);
                //UIDBG("window:%p\n", window);
                switch (miceEvent.type) {
                case MICE_EVENT_PRESS:
                    if (miceEvent.button == MICE_BUTTON_LEFT) {
                        oldWindow = focusWindow();
                        setFocusWindow(window);
                        if (window)
                            unionWindow(&dirtyRect, window);

                    }
                    break;
                case MICE_EVENT_MOVE:
                    unionRect(&dirtyRect, &cursorRect);
                    cursorX = MAX(0, MIN(WIDTH, cursorX + miceEvent.deltaX*4));
                    cursorY = MAX(0, MIN(HEIGHT, cursorY + miceEvent.deltaY*4));
                    initRect(&cursorRect, cursorX - CURSOR_SIZE, cursorY - CURSOR_SIZE, CURSOR_SIZE*2, CURSOR_SIZE*2);
                    unionRect(&dirtyRect, &cursorRect);
                    break;
                }
                if (window) {
                    miceEventNotify->type = OZUI_EVENT_MICE;
                    miceEventNotify->id = windowId(window);
                    miceEventNotify->miceEvent.type = miceEvent.type;
                    miceEventNotify->miceEvent.x = cursorX;
                    miceEventNotify->miceEvent.y = cursorY;
                    miceEventNotify->miceEvent.button = miceEvent.button;
                    notifyApp(window->app, miceEventNotify, sizeof(struct OzUIMiceEventNotify));
                }
                OzReadAsync(miceFD, sizeof(struct MiceEvent), &miceEvent);
            } else if (ioEventPtr->fd == kbdFD) {
                window = focusWindow();
                if (window) {
                    keyEventNotify->type = OZUI_EVENT_KEY;
                    keyEventNotify->id = windowId(window);
                    keyEventNotify->keyEvent.type = keyEvent.type;
                    keyEventNotify->keyEvent.code = keyEvent.code;
                    keyEventNotify->keyEvent.mask = updateKeyMask(&keyEvent);
                    notifyApp(window->app, keyEventNotify, sizeof(struct OzUIKeyEventNotify));
                }
                OzReadAsync(kbdFD, sizeof(struct KeyEvent), &keyEvent);
            }
            break;
        case OZUI_EVENT_CREATE_WINDOW:
            createWindowRequest = (struct OzUICreateWindowRequest*)buf;
            window = createWindow(hdr.pid, createWindowRequest->width, createWindowRequest->height, createWindowRequest->flags);
            createWindowReply->id = windowId(window);
            copyRect(&createWindowReply->screenRect, &window->screenRect);
            OzReply(hdr.pid, createWindowReply, sizeof(struct OzUICreateWindowReply));
            unionWindow(&dirtyRect, window);
            break;
        case OZUI_EVENT_DESTROY_WINDOW:
            destroyWindowRequest = (struct OzUIDestroyWindowRequest*)buf;
            window = getWindowById(destroyWindowRequest->id);
            destroyWindow(window);
            destroyWindowReply->ret = 0;
            OzReply(hdr.pid, destroyWindowReply, sizeof(struct OzUIDestroyWindowReply));
            unionWindow(&dirtyRect, window);
            break;
        case OZUI_EVENT_MOVE_WINDOW:
            moveWindowRequest = (struct OzUIMoveWindowRequest*)buf;
            window = getWindowById(moveWindowRequest->id);
            unionWindow(&dirtyRect, window);
            moveWindow(window, moveWindowRequest->x, moveWindowRequest->y);
            unionWindow(&dirtyRect, window);
            moveWindowReply->ret = 0;
            OzReply(hdr.pid, moveWindowReply, sizeof(struct OzUIMoveWindowReply));
            break;
        case OZUI_EVENT_NEXT_EVENT:
            app = getApp(hdr.pid);
            pollAppEvent(app);
            break;
        case OZUI_EVENT_DRAW_RECTANGLE:
            drawRectangleRequest = (struct OzUIWindowDrawRectangleRequest*)buf;
            window = getWindowById(drawRectangleRequest->id);
            drawRectangle(window, &drawRectangleRequest->clipRect, &drawRectangleRequest->rect, &drawRectangleRequest->lineStyle, &drawRectangleRequest->fillStyle);
            //unionWindow(&dirtyRect, window);
            unionWindowRect(&dirtyRect, window, &drawRectangleRequest->clipRect);
            drawRectangleReply->ret = 0;
            OzReply(hdr.pid, drawRectangleReply, sizeof(struct OzUIWindowDrawRectangleReply));
            break;
        case OZUI_EVENT_DRAW_LINE:
            drawLineRequest = (struct OzUIWindowDrawLineRequest*)buf;
            window = getWindowById(drawLineRequest->id);
            drawLine(window, &drawLineRequest->clipRect, &drawLineRequest->p1, &drawLineRequest->p2, &drawLineRequest->lineStyle);
            //unionWindow(&dirtyRect, window);
            unionWindowRect(&dirtyRect, window, &drawLineRequest->clipRect);
            drawLineReply->ret = 0;
            OzReply(hdr.pid, drawLineReply, sizeof(struct OzUIWindowDrawLineReply));
            break;
        case OZUI_EVENT_DRAW_TEXT:
            drawTextRequest = (struct OzUIWindowDrawTextRequest*)buf;
            window = getWindowById(drawTextRequest->id);
            drawText(window, &drawTextRequest->clipRect, &drawTextRequest->tlc, drawTextRequest->text, &drawTextRequest->lineStyle, &drawTextReply->layout);
            //unionWindow(&dirtyRect, window);
            unionWindowRect(&dirtyRect, window, &drawTextRequest->clipRect);
            drawTextReply->ret = 0;
            //drawTextReplyLen = SIZE_OZUI_WINDOW_DRAW_TEXT_REPLY(drawTextReply);
            OzReply(hdr.pid, drawTextReply, SIZE_OZUI_WINDOW_DRAW_TEXT_REPLY(drawTextReply));
            //OzReply(hdr.pid, drawTextReply,  200);
            break;
        case OZUI_EVENT_QUERY_TEXT_LAYOUT:
            layoutTextRequest = (struct OzUIWindowQueryTextLayoutRequest*)buf;
            window = getWindowById(layoutTextRequest->id);
            windowQueryTextLayout(window, &layoutTextRequest->clipRect, &layoutTextRequest->tlc, layoutTextRequest->text, &layoutTextReply->layout);
            layoutTextReply->ret = 0;
            OzReply(hdr.pid, layoutTextReply, SIZE_OZUI_WINDOW_QUERY_TEXT_LAYOUT_REPLY(layoutTextReply));
            break;
        case OZUI_EVENT_DRAW_TEXT_LAYOUT:
            drawTextLayoutRequest = (struct OzUIWindowDrawTextLayoutRequest*)buf;
            window = getWindowById(drawTextLayoutRequest->id);
            windowDrawTextLayout(window, &drawTextLayoutRequest->clipRect, &drawTextLayoutRequest->tlc, &drawTextLayoutRequest->lineStyle, &drawTextLayoutRequest->layout);
            unionWindowRect(&dirtyRect, window, &drawTextLayoutRequest->clipRect);
            drawTextLayoutReply->ret = 0;
            OzReply(hdr.pid, drawTextLayoutReply, sizeof(struct OzUIWindowDrawTextLayoutReply));
            break;
        }
    }
    return 0;
}

