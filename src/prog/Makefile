PREFIX = /home/ora/cross
QUIET = #@
LD32 = ld
CC = x86_64-pc-orzos-gcc
AR = x86_64-pc-orzos-ar
LD = x86_64-pc-orzos-ld
NM = x86_64-pc-orzos-nm
AS = nasm
INSTALL = install 

BASE_CFLAGS = --sysroot=$(PREFIX) -I/home/ora/cross/include -I/home/ora/cross/x86_64-pc-orzos/include

FREETYPE2_CFLAGS = -I$(PREFIX)/include/freetype2
FREETYPE2_LIBS = -lfreetype -lz

CAIRO_CFLAGS = -I$(PREFIX)/include/cairo -I/home/ora/cross/include/pixman-1 
CAIRO_LIBS = -L$(PREFIX)/lib/gcc/x86_64-pc-orzos/4.5.0 -lcairo -lpixman-1 -lpng -lgcc

PROG_CFLAGS=-ffreestanding -fno-stack-protector -mcmodel=large -mno-red-zone -Ix86emu -Wall $(BASE_CFLAGS) $(FREETYPE2_CFLAGS) $(CAIRO_CFLAGS) -g

PROG_LDFLAGS=-T$(PREFIX)/ldscripts/prog.ld -m orzos_x86_64 -z max-page-size=0x1000 -L$(PREFIX)/lib -L$(PREFIX)/x86_64-pc-orzos/lib $(FREETYPE2_LIBS) $(CAIRO_LIBS) $(FREETYPE2_LIBS) -lc -lm

ASFLAGS=-felf64

all:init uiserver uiclient

init_SRCS = init.o

uiserver_SRCS = uiserver.o window.o rect.o guiconfig.o app.o drawops.o layout.o utf8.o

uiclient_SRCS = uiclient.o uiproto.o uilabel.o uibutton.o uiapp.o uiwindow.o uiwidget.o rect.o guiconfig.o

%.o:%.c
	@echo "CC	$@"
	$(QUIET)$(CC) $(PROG_CFLAGS) -c $< -o $@

init:$(init_SRCS)
	@echo "LD	$@"
	$(QUIET)$(LD) -o $@ $+ $(PROG_LDFLAGS)

uiserver:$(uiserver_SRCS)
	@echo "LD	$@"
	$(QUIET)$(LD) -o $@ $+ $(PROG_LDFLAGS)

uiclient:$(uiclient_SRCS)
	@echo "LD	$@"
	$(QUIET)$(LD) -o $@ $+ $(PROG_LDFLAGS)

clean:
	$(QUIET)-rm *.o init uiserver uiclient

ctags:
	$(QUIET)ctags -R --c-kinds=+p --fields=+S . 
