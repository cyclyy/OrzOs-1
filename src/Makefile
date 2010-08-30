CORE_SOURCES = boot.o              \
               main.o              \
               common.o            \
               screen.o            \
               dt_flush.o          \
               desc_table.o        \
               isr.o               \
               interrupt.o         \
               timer.o             \
               task.o              \
               process.o           \
               kheap.o             \
               paging.o            \
               order_array.o       \
               syscall.o           \
               syscalls.o          \
               vfs.o               \
               file.o              \
               module.o            \
               vm.o                \
               dev.o               \
               disk.o              \
               cpio.o              \
               fs/ramfs.o          \

MOD_SOURCES = testmod.o            \
              i8042.o              \
              pci.o                \
              ide.o                \
              fs/ext2fs.o          \

PROG_SOURCES = prog/init.o         \
               syscall.o           \

CFLAGS=-nostdlib -nostdinc -fno-builtin -fno-stack-protector -Wall -g -I.
CORE_LDFLAGS=-Tcore.ld -M=allsymbol.map
PROG_LDFLAGS=-Tprog.ld
ASFLAGS=-felf

all: $(CORE_SOURCES) $(MOD_SOURCES) $(PROG_SOURCES) core_link prog_link

clean:
	-rm *.o ../util/*.o prog/*.o prog/init fs/*.o kernel allsymbol.map global.map

core_link:
	ld $(CORE_LDFLAGS) -o kernel $(CORE_SOURCES)
	nm ./kernel > global.map

prog_link:
	ld $(PROG_LDFLAGS) -o prog/init $(PROG_SOURCES)

.s.o:
	nasm $(ASFLAGS) $< 
