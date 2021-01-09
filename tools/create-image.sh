#!/bin/bash
DISK=dbos-disk.img
BOOT=./boot

rm -rf $DISK
# Create a 1GiB blank disk image.
dd if=/dev/zero of=$DISK bs=256M count=1
echo "*********************************"
echo "*************disk image created**"
echo "*********************************"

cat config/parted.conf | parted $DISK
echo "*********************************"
echo "*********disk image partitioned**"
echo "*********************************"

LOOPRAW=`losetup -f`
sudo losetup $LOOPRAW $DISK
echo "*********************************"
echo "*****image attached to loop dev**"
echo "*********************************"

TMP=`sudo kpartx -av $DISK`
TMP2=${TMP/add map /}
LOOP=${TMP2%%p1 *}
LOOPDEV=/dev/${LOOP}
LOOPMAP=/dev/mapper/${LOOP}p1
sleep 1s

sudo mkfs.ext2 ${LOOPMAP}
echo "*********************************"
echo "*******ext2 filesystem created **"
echo "*********************************"
sleep 1s
sudo mount ${LOOPMAP} /mnt
echo "*********************************"
echo "***********loop device mounted **"
echo "*********************************"

sudo cp -r sysroot/* /mnt/
echo "*********************************"
echo "****directory structure copied **"
echo "*********************************"

sudo grub-install --target=i386-pc --boot-directory=/mnt/boot $LOOPRAW
echo "*********************************"
echo "********grub installed to disk **"
echo "*********************************"

sudo umount /mnt
echo "*********************************"
echo "**********loop device unmounted**"
echo "*********************************"

sudo kpartx -d ${LOOPMAP}
echo "*********************************"
echo "*****partition mappings deleted**"
echo "*********************************"

sudo dmsetup remove ${LOOPMAP}
echo "*********************************"
echo "*********logical volume removed**"
echo "*********************************"

sudo losetup -d ${LOOPDEV}
sudo losetup -d ${LOOPRAW}
echo "*********************************"
echo "***********loop devices cleared**"
echo "*********************************"
