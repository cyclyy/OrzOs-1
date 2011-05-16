#!/bin/bash

# run_bochs.sh
# mounts the correct loopback device, runs bochs, then unmounts.

sudo ./update_image.sh
qemu-system-x86_64 -s -S -fda floppy.img -m 1024 -monitor stdio
