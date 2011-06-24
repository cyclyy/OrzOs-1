#!/bin/bash

MEM=256

sudo ./update_image.sh
qemu-system-x86_64 -s -S -fda floppy.img -m $MEM -monitor stdio -vga std
#qemu-system-x86_64 -s -S -fda floppy.img -m $MEM -chardev stdio,id=seabios -device isa-debugcon,iobase=0xe9,chardev=seabios -vga std
