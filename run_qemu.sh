#!/bin/bash

MEM=32

sudo ./update_image.sh
qemu-system-x86_64 -s -S -fda floppy.img -m $MEM -monitor stdio
