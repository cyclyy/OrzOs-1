#!/bin/bash

MEM=256

sudo ./update_image.sh
qemu-system-x86_64 -s -S -fda floppy.img -m $MEM -monitor stdio
