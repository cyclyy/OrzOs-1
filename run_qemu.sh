#!/bin/bash

MEM=256

sudo ./update_image.sh
#qemu-system-x86_64 -s -S -boot order=a -fda floppy.img -hda c.img -m $MEM -monitor stdio -vga std
#qemu-system-x86_64 -s -S -fda floppy.img -m $MEM -chardev stdio,id=seabios -device isa-debugcon,iobase=0xe9,chardev=seabios -vga std
qemu-system-x86_64 -d cpu_reset -s -S -boot order=a -fda floppy.img -hda c.img -m $MEM -vga std -monitor vc -debugcon stdio 
