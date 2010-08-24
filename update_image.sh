#! /bin/bash

rm util/init
rm util/testmod.o
rm util/i8042.o
rm util/pci.o
cd src
make 
cd ..
cp src/prog/init util/
cp src/testmod.o util/
cp src/i8042.o util/
cp src/pci.o util/
cd util
./make_initrd testmod.o pci.o i8042.o init
cd ..
ls c.img || bximage -hd -mode=growing -size=10 -q c.img
sudo losetup /dev/loop0 floppy.img
mkdir /tmp/floppy
sudo mount /dev/loop0 /tmp/floppy
sudo cp src/kernel /tmp/floppy/kernel
sudo cp util/initrd.img /tmp/floppy/initrd.img
sudo umount /dev/loop0
sudo losetup -d /dev/loop0 
