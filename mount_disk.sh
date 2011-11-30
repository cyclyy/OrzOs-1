#!/bin/sh

mkdir /tmp/c
sudo losetup -d /dev/loop5
sudo losetup -d /dev/loop6
sudo losetup -o 32256 /dev/loop5 c.img
sudo mount -t ext2 /dev/loop5 /tmp/c
