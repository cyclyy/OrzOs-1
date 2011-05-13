#! /bin/bash

sudo losetup /dev/loop2 floppy.img
sudo mount /dev/loop2 /tmp/floppy
