LD32 = ld
CC = x86_64-pc-elf-gcc
AR = x86_64-pc-elf-ar
LD = x86_64-pc-elf-ld
NM = x86_64-pc-elf-nm

CORE_SOURCES = loader64.o main.o              \
               util.o              \
               screen.o        

MOD_SOURCES = testmod.o            \
              i8042.o              \
              pci.o                \
              ide.o                \
              fs/ext2fs.o          \

PROG_SOURCES = prog/init.o         \
               syscall.o           \

CFLAGS=-m64 -ffreestanding -nostdlib -nodefaultlibs -nostdinc -fno-builtin -fno-stack-protector -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -mno-sse3 -mno-3dnow -Wall -I.
CORE_LDFLAGS=-Tcore.ld -M=allsymbol.map -z max-page-size=0x1000
PROG_LDFLAGS=-Tprog.ld
ASFLAGS=-felf64

# all: $(CORE_SOURCES) $(MOD_SOURCES) $(PROG_SOURCES) core_link prog_link
all: $(CORE_SOURCES) core_link loader32

clean:
	-rm *.o ../util/*.o prog/*.o prog/init fs/*.o kernel allsymbol.map global.map

loader32: loader32.s
	nasm -f elf32 loader32.s

core_link:
	$(LD) $(CORE_LDFLAGS) -o kernel $(CORE_SOURCES)
	$(NM) ./kernel > global.map

prog_link:
	$(LD) $(PROG_LDFLAGS) -o prog/init $(PROG_SOURCES)

.s.o:
	nasm $(ASFLAGS) $< 

ctags:
	ctags -R .
