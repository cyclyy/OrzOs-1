QUIET = @
LD32 = ld
# CC = x86_64-pc-elf-gcc
CC = x86_64-pc-elf-gcc
AR = x86_64-pc-elf-ar
LD = x86_64-pc-elf-ld
NM = x86_64-pc-elf-nm

X86EMU_SOURCES = x86emu/decode.o x86emu/fpu.o x86emu/ops.o x86emu/ops2.o x86emu/prim_ops.o x86emu/sys.o

CORE_SOURCES = main.o util.o kmm.o vmm.o bootinfo.o dtable.o flushdtable.o isr.o \
			interrupt.o tlsf.o vfs.o screen.o task.o taskasm.o paging.o \
			elfloader.o waitqueue.o schedule.o semaphore.o mutex.o message.o \
			ksyscall.o device.o i8042.o program.o except.o usermemcopy.o \
			handle.o fs/bootfs.o fs/devfs.o \
			syscall/ozipc.o syscall/oztask.o syscall/ozfs.o \
			rmi.o video/vbe.o

MOD_SOURCES = testmod.o            \
              i8042.o              \
              pci.o                \
              ide.o                \
              ext2fs.o          \

PROG_SOURCES = prog/init.o  prog/server.o prog/client.o syscall.o 

CFLAGS=-ffreestanding -nostdlib -nostdinc -fno-builtin -fno-stack-protector -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -mno-sse3 -mno-3dnow -Ix86emu -DNO_SYS_HEADERS -Wall -I. -g
CORE_LDFLAGS=-Tcore.ld -M=allsymbol.map -z max-page-size=0x1000
PROG_LDFLAGS=-Tprog.ld -z max-page-size=0x1000
ASFLAGS=-felf64

# all: $(CORE_SOURCES) $(MOD_SOURCES) $(PROG_SOURCES) libx86emu.a core_link prog_link
all: $(CORE_SOURCES) $(PROG_SOURCES) $(X86EMU_SOURCES) libx86emu.a core_link prog_link loader32 ctags

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
	$(QUIET)$(LD) $(CORE_LDFLAGS) -melf_x86_64 -o kernel $(CORE_SOURCES) libx86emu.a
	$(QUIET)$(NM) ./kernel > global.map

prog_link: $(PROG_SOURCES)
	@echo "LD	init"
	$(QUIET)$(LD) $(PROG_LDFLAGS) -o prog/init prog/init.o syscall.o
	@echo "LD	server"
	$(QUIET)$(LD) $(PROG_LDFLAGS) -o prog/server prog/server.o syscall.o
	@echo "LD	client"
	$(QUIET)$(LD) $(PROG_LDFLAGS) -o prog/client prog/client.o syscall.o

%.o:%.s
	@echo "AS	$@"
	$(QUIET)nasm $(ASFLAGS) $< 

%.o:%.c
	@echo "CC	$@"
	$(QUIET)$(CC) $(CFLAGS) -c $< -o $@

ctags:
	$(QUIET)ctags -R .
