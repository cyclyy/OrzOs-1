#! /bin/bash

sudo losetup /dev/loop0 floppy.img
sudo mount /dev/loop0 /tmp/floppy
