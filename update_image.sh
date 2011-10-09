#! /bin/bash

rm  initrd/init
cp src/prog/init initrd/
rm  initrd/server
cp src/prog/server initrd/
rm  initrd/client
cp src/prog/client initrd/
#rm  initrd/uiserver
#cp src/prog/uiserver initrd/
rm  initrd/init
cp src/prog/init initrd/
rm initrd.tar
tar cvf initrd.tar initrd/
mv initrd.tar initrd.img
ls c.img 2>/dev/null || bximage -hd -mode=flat -size=100 -q c.img
sudo losetup /dev/loop20 floppy.img
ls /tmp/floppy 1>/dev/null || mkdir /tmp/floppy
sudo mount /dev/loop20 /tmp/floppy
sudo cp src/loader32 /tmp/floppy/loader32
sudo cp src/kernel /tmp/floppy/kernel
sudo cp initrd.img /tmp/floppy/initrd.img
sudo umount /dev/loop20
sudo losetup -d /dev/loop20 

mkdir /tmp/c
sudo losetup -d /dev/loop5
sudo losetup -o 32256 /dev/loop5 c.img
sudo mount -t ext2 /dev/loop5 /tmp/c
rm /tmp/c/uiserver
cp initrd/a.txt /tmp/c/t1.txt -v
cp src/prog/uiserver /tmp/c -v
cp zenhei.ttc /tmp/c -v
cp wallpaper.png /tmp/c -v
cp cursor.png /tmp/c -v
sudo umount /tmp/c
sudo losetup -d /dev/loop5
