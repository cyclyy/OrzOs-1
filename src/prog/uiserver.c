#include "uidef.h"
#include "uiserver.h"
#include "uiproto.h"
#include "syscall.h"
#include <stdlib.h>
#include <string.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <cairo.h>
#include <cairo-ft.h>

#define WIDTH 800
#define HEIGHT 600

struct Pixel
{
    char r:5;
    char g:6;
    char b:5;
} __attribute__((packed));

struct Pixel *frameBuffer;

FT_Library library;
FT_Face face;

s64int parseMessage(char *buf, u64int size)
{
    struct UIRequestHeader *header;
    header = (struct UIRequestHeader*)buf;
    return header->type;
}

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


void initFontRender()
{
    wchar_t *msg = L"我";
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
    surface = cairo_image_surface_create_for_data(frameBuffer, CAIRO_FORMAT_RGB16_565,WIDTH,HEIGHT,WIDTH*2);
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
    FILE *f;
    f = fopen("Device:/Debug", "w");
    fprintf(f,"我width:%lf,height:%lf,x_bear:%lf,y_bear:%lf,x_adv:%lf,y_adv:%lf\n",te.width,te.height,te.x_bearing,te.y_bearing,te.x_advance,te.y_advance);
    cairo_font_extents_t fex;
    cairo_font_extents(cr, &fex);
    fprintf(f,"ascent:%lf,descent:%lf,height:%lf,max_x_adv:%lf,max_y_adv:%lf\n",fex.ascent,fex.descent,fex.height,fex.max_x_advance,fex.max_y_advance);
    fclose(f);
    //cairo_rectangle(cr,0,0,1,1);
    //cairo_fill(cr);
    cairo_surface_flush(surface);
    //show_image();
}

int main()
{
    s64int serverId, msgId, display;
    char *recvBuf, *replyBuf;
    u64int n;
    struct MessageInfo mi;
    struct UIInitRequest *initReq;
    struct UIConsoleWriteRequest *conWriteReq;
    struct UIInitReply *initReply;
    struct UIConsoleWriteReply *conWriteReply;

    display = openDisplay();
    initFontRender();
    initCairo();

    n = 1000;
    recvBuf = (char*)malloc(n);
    replyBuf = (char*)malloc(n);
    initReq = (struct UIInitRequest*)recvBuf;
    conWriteReq = (struct UIConsoleWriteRequest*)recvBuf;
    initReply = (struct UIInitReply*)replyBuf;
    conWriteReply = (struct UIConsoleWriteReply*)replyBuf;
    serverId = OzCreateServer(UI_PORT);
    while ((msgId = OzReceive(serverId, n, recvBuf, &mi)) >= 0) {
        switch (parseMessage(recvBuf, mi.srcSize)) {
            case UI_REQUEST_INIT:
                initReply->type = UI_REPLY_INIT;
                initReply->retCode = 0;
                initReply->clientId = 1;
                if (msgId) {
                    OzReply(msgId,sizeof(struct UIInitRequest),initReply);
                }
                break;
            case UI_REQUEST_CONSOLE_WRITE:
                conWriteReply->type = UI_REPLY_CONSOLE_WRITE;
                conWriteReply->retCode = 0;
                if (msgId) {
                    OzReply(msgId,sizeof(struct UIConsoleWriteReply),conWriteReply);
                }
                break;
            default:
                ;
        }

    }
    return 0;
}

