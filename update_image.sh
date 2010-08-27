#! /bin/bash

rm initrd/boot/init
rm initrd/boot/modules/testmod.o
rm initrd/boot/modules/i8042.o
rm initrd/boot/modules/pci.o
rm initrd/boot/modules/ide.o
cd src
make 
cd ..
mkdir initrd/boot 2>/dev/null
mkdir initrd/boot/modules 2>/dev/null
cp src/prog/init initrd/boot/
cp src/testmod.o initrd/boot/modules/
cp src/i8042.o initrd/boot/modules/
cp src/pci.o initrd/boot/modules/
cp src/ide.o initrd/boot/modules/
rm initrd.img
cd initrd
find -depth -print | sort | cpio -ov > ../initrd.img
cd ..
ls c.img 2>/dev/null || bximage -hd -mode=flat -size=10 -q c.img
sudo losetup /dev/loop0 floppy.img
ls /tmp/floppy 1>/dev/null || mkdir /tmp/floppy
sudo mount /dev/loop0 /tmp/floppy
sudo cp src/kernel /tmp/floppy/kernel
sudo cp initrd.img /tmp/floppy/initrd.img
sudo umount /dev/loop0
sudo losetup -d /dev/loop0 
