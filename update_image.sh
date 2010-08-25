#! /bin/bash

rm util/init
rm util/testmod.o
rm util/i8042.o
rm util/pci.o
rm util/ide.o
cd src
make 
cd ..
cp src/prog/init util/
cp src/testmod.o util/
cp src/i8042.o util/
cp src/pci.o util/
cp src/ide.o util/
cd util
./make_initrd testmod.o pci.o ide.o i8042.o init
cd ..
ls c.img || bximage -hd -mode=flat -size=10 -q c.img
sudo losetup /dev/loop0 floppy.img
ls /tmp/floppy 1>/dev/null || mkdir /tmp/floppy
sudo mount /dev/loop0 /tmp/floppy
sudo cp src/kernel /tmp/floppy/kernel
sudo cp util/initrd.img /tmp/floppy/initrd.img
sudo umount /dev/loop0
sudo losetup -d /dev/loop0 
