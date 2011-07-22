#include "uidef.h"
#include "uiserver.h"
#include "uiproto.h"
#include "syscall.h"
#include <stdlib.h>
#include <string.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <cairo.h>

#define WIDTH 800
#define HEIGHT 600

struct Pixel
{
    char r,g,b;
} __attribute__((packed));

struct Pixel *frameBuffer;

FT_Library library;

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
    mi.cellBits = 24;
    OzIoControl(fd, DISPLAY_IOCTL_SET_MODE, sizeof(struct DisplayModeInfo), &mi);
    frameBuffer = (struct Pixel*)OzMap(fd, 0, 0, 0);
    int i,j,idx;
    for (i=0; i<HEIGHT; i++) {
        for (j=0; j<WIDTH; j++) {
            idx = i*WIDTH + j;
            frameBuffer[idx].r = 0;
            frameBuffer[idx].g = 255;
            frameBuffer[idx].b = 0;
        }
    }
    return fd;
}

unsigned char image[HEIGHT][WIDTH];

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
    FT_Face face;
    FT_GlyphSlot slot;
    FT_Matrix matrix;
    FT_Vector pen;
    FT_Error error;
    error = FT_Init_FreeType(&library);
    error = FT_New_Face(library, "C:/zenhei.ttc", 0, &face);
    error = FT_Set_Char_Size(face, 300*64, 0, 100, 0);
    slot = face->glyph;
    //error = FT_Load_Char(face, 0x56e7, FT_LOAD_RENDER);
    error = FT_Load_Char(face, msg[0], FT_LOAD_RENDER);
    drawBitmap(&slot->bitmap, slot->bitmap_left, HEIGHT - slot->bitmap_top - 100);
    show_image();
}

void initCairo()
{
    cairo_surface_t *surface;
    surface = cairo_image_surface_create(CAIRO_FORMAT_RGB16_565,WIDTH, HEIGHT);
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

