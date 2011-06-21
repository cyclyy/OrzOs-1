#! /bin/bash

rm  initrd/init
cp src/prog/init initrd/
rm  initrd/server
cp src/prog/server initrd/
rm  initrd/client
cp src/prog/client initrd/

rm  initrd/init
cp src/prog/init initrd/
rm initrd.tar
tar cvf initrd.tar initrd/
ls c.img 2>/dev/null || bximage -hd -mode=flat -size=10 -q c.img
sudo losetup /dev/loop2 floppy.img
ls /tmp/floppy 1>/dev/null || mkdir /tmp/floppy
sudo mount /dev/loop2 /tmp/floppy
sudo cp src/loader32 /tmp/floppy/loader32
sudo cp src/kernel /tmp/floppy/kernel
sudo cp initrd.tar /tmp/floppy/initrd.img
sudo umount /dev/loop2
sudo losetup -d /dev/loop2 
