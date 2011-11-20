PREFIX = /home/ora/cross
QUIET = @
LD32 = ld
CC = x86_64-pc-orzos-gcc
AR = x86_64-pc-orzos-ar
LD = x86_64-pc-orzos-ld
NM = x86_64-pc-orzos-nm
AS = nasm
INSTALL = install 

X86EMU_SOURCES = x86emu/decode.o x86emu/fpu.o x86emu/ops.o x86emu/ops2.o x86emu/prim_ops.o x86emu/sys.o

INSTALL_HEADERS = syscall.h sysdef.h mice.h key.h event.h $(wildcard libc/*.h)
INSTALL_LDSCRIPTS = prog.ld core.ld
INSTALL_LIBRARIES = liborzos.a libx86emu.a

CORE_C_SOURCES = main.o util.o kmm.o vmm.o bootinfo.o cpu.o dtable.o \
			interrupt.o tlsf.o vfs.o debugcon.o task.o paging.o \
			elfloader.o waitqueue.o schedule.o semaphore.o mutex.o message.o \
			ksyscall.o device.o i8042.o program.o except.o \
			handle.o fs/bootfs.o fs/devfs.o fs/ext2fs.o \
			syscall/ozipc.o syscall/oztask.o syscall/oztime.o syscall/ozfs.o \
			rmi.o video/display.o video/vbe.o \
			pci.o ide.o disk.o cmos.o debugdev.o timer.o iorequest.o

CORE_ASM_SOURCES = flushdtable.o isr.o taskasm.o usermemcopy.o

CORE_SOURCES = $(CORE_C_SOURCES) $(CORE_ASM_SOURCES)

MOD_SOURCES = testmod.o            \
              i8042.o              \
              pci.o                \
              ide.o                \
              ext2fs.o             \

CORE_CFLAGS=-ffreestanding -nostdlib -nostdinc -fno-builtin -fno-stack-protector -mcmodel=kernel -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -mno-sse3 -mno-3dnow -Ix86emu -DNO_SYS_HEADERS -Wall -I. -g 

CORE_LDFLAGS=-Tcore.ld -m orzos_x86_64 -M=allsymbol.map -z max-page-size=0x1000

ASFLAGS=-felf64

###################################################################
###################################################################

all: kernel install prog ctags
kernel: $(CORE_C_SOURCES) $(CORE_ASM_SOURCES) $(X86EMU_SOURCES) libx86emu.a core_link loader32

clean:
	$(QUIET)-rm *.o ../util/*.o prog/*.o prog/init video/*.o fs/*.o syscall/*.o x86emu/*.o kernel allsymbol.map global.map

%.d: %.c
	@set -e; rm -f $@; \
	$(CC) -I. -Ix86emu -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed -r 's,($*)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

loader32: loader32.s
	@echo "AS	$@"
	$(QUIET)nasm -f bin loader32.s -o loader32

liborzos.a:syscall.o
	@echo "LD	orzos"
	$(QUIET)$(AR) r $@ $<

libx86emu.a: $(X86EMU_SOURCES)
	@echo "LD	x86emu"
	$(QUIET)$(AR) r $@ $+ 1>&/dev/null

core_link:libx86emu.a
	@echo "LD	kernel"
	$(QUIET)$(LD) $(CORE_LDFLAGS) -o kernel $(CORE_SOURCES) libx86emu.a
	$(QUIET)$(NM) ./kernel > global.map

prog:FORCE
	@make -C prog

$(CORE_ASM_SOURCES):%.o:%.s
	@echo "AS	$@"
	$(QUIET)nasm $(ASFLAGS) $< 

$(CORE_C_SOURCES) $(X86EMU_SOURCES):%.o:%.c
	@echo "CC	$@"
	$(QUIET)$(CC) $(CORE_CFLAGS) -c $< -o $@

install:install_headers install_ldscripts install_libraries

install_headers:$(INSTALL_HEADERS)
	@echo "INSTALL  $?"
	$(QUIET)$(INSTALL) -d $(PREFIX)/include/os
	$(QUIET)$(INSTALL) -t $(PREFIX)/include/os $?

install_ldscripts:$(INSTALL_LDSCRIPTS)
	@echo "INSTALL $?"
	$(QUIET)$(INSTALL) -d $(PREFIX)/ldscripts
	$(QUIET)$(INSTALL) -t $(PREFIX)/ldscripts $?

install_libraries:$(INSTALL_LIBRARIES)
	@echo "INSTALL $?"
	$(QUIET)$(INSTALL) -d $(PREFIX)/lib
	$(QUIET)$(INSTALL) -t $(PREFIX)/lib $?

ctags:
	$(QUIET)ctags -R --c-kinds=+p --fields=+S . 

include $(CORE_C_SOURCES:.o=.d)

FORCE:
