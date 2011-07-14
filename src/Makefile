QUIET = @
LD32 = ld
# CC = x86_64-pc-elf-gcc
CC = x86_64-pc-orzos-gcc
AR = x86_64-pc-orzos-ar
LD = x86_64-pc-orzos-ld
NM = x86_64-pc-orzos-nm
AS = nasm
X86EMU_SOURCES = x86emu/decode.o x86emu/fpu.o x86emu/ops.o x86emu/ops2.o x86emu/prim_ops.o x86emu/sys.o

CORE_C_SOURCES = main.o util.o kmm.o vmm.o bootinfo.o dtable.o \
			interrupt.o tlsf.o vfs.o screen.o task.o paging.o \
			elfloader.o waitqueue.o schedule.o semaphore.o mutex.o message.o \
			ksyscall.o device.o i8042.o program.o except.o \
			handle.o fs/bootfs.o fs/devfs.o fs/ext2fs.o \
			syscall/ozipc.o syscall/oztask.o syscall/ozfs.o \
			rmi.o video/display.o video/vbe.o \
			pci.o ide.o disk.o

CORE_ASM_SOURCES = flushdtable.o isr.o taskasm.o usermemcopy.o

CORE_SOURCES = $(CORE_C_SOURCES) $(CORE_ASM_SOURCES)

MOD_SOURCES = testmod.o            \
              i8042.o              \
              pci.o                \
              ide.o                \
              ext2fs.o          \

PROG_SOURCES = prog/uiproto.o prog/uiserver.o prog/init.o prog/server.o prog/client.o syscall.o 

CORE_CFLAGS=-ffreestanding -nostdlib -nostdinc -fno-builtin -fno-stack-protector -mcmodel=kernel -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -mno-sse3 -mno-3dnow -Ix86emu -DNO_SYS_HEADERS -Wall -I. -g
PROG_CFLAGS=-ffreestanding -fno-stack-protector -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -mno-sse3 -mno-3dnow -Ix86emu -DNO_SYS_HEADERS -Wall -I. -g
CORE_LDFLAGS=-Tcore.ld -m orzos_x86_64 -M=allsymbol.map -z max-page-size=0x1000
PROG_LDFLAGS=-Tprog.ld -m orzos_x86_64 -z max-page-size=0x1000 -lc
ASFLAGS=-felf64

# all: $(CORE_SOURCES) $(MOD_SOURCES) $(PROG_SOURCES) libx86emu.a core_link prog_link
all: $(CORE_C_SOURCES) $(CORE_ASM_SOURCES) $(PROG_SOURCES) $(X86EMU_SOURCES) libx86emu.a core_link prog_link loader32 ctags

clean:
	$(QUIET)-rm *.o ../util/*.o prog/*.o prog/init video/*.o fs/*.o syscall/*.o x86emu/*.o kernel allsymbol.map global.map

loader32: loader32.s
	@echo "AS	$@"
	$(QUIET)nasm -f bin loader32.s -o loader32

libx86emu.a: $(X86EMU_SOURCES)
	@echo "LD	x86emu"
	$(QUIET)$(AR) r $@ $+ 1>&/dev/null

core_link:libx86emu.a
	@echo "LD	kernel"
	$(QUIET)$(LD) $(CORE_LDFLAGS) -o kernel $(CORE_SOURCES) libx86emu.a
	$(QUIET)$(NM) ./kernel > global.map

prog_link: 
	@echo "LD	init"
	$(QUIET)$(LD) -o prog/init prog/init.o syscall.o $(PROG_LDFLAGS)
	@echo "LD	uiserver"
	$(QUIET)$(LD) -o prog/uiserver prog/uiserver.o prog/uiproto.o syscall.o $(PROG_LDFLAGS)

$(CORE_ASM_SOURCES):%.o:%.s
	@echo "AS	$@"
	$(QUIET)nasm $(ASFLAGS) $< 

$(CORE_C_SOURCES) $(X86EMU_SOURCES):%.o:%.c
	@echo "CC	$@"
	$(QUIET)$(CC) $(CORE_CFLAGS) -c $< -o $@

$(PROG_SOURCES):%.o:%.c
	@echo "CC	$@"
	$(QUIET)$(CC) $(PROG_CFLAGS) -c $< -o $@

ctags:
	$(QUIET)ctags -R .
