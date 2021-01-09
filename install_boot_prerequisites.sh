#!/bin/bash
# grab the directory path in which we have the build script - ..../build_tools/
WD="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

ERR=$(readlink -f build_tools/error.sh)
SUC=$(readlink -f build_tools/success.sh)
INF=$(readlink -f build_tools/info.sh)

$INF "PROBING FOR NASM..."
sudo apt-get -y install nasm &> $WD/error.log || $ERR "ERRORS INSTALLING NASM"
$SUC "DONE!"

$INF "PROBING FOR GENEXT2FS"
sudo apt-get -y install genext2fs &> $WD/error.log || $ERR "ERRORS INSTALLING GENEXT2FS"
$SUC "DONE!"

$INF "PROBING FOR GRUB"
sudo apt-get -y install grub2-common &> $WD/error.log || $ERR "ERRORS INSTALLING GRUB"
sudo apt-get -y install xorriso &> $WD/error.log || $ERR "ERRORS INSTALLING XORRISO"
sudo apt-get -y install grub-pc-bin &> $WD/error.log || $ERR "ERRORS INSTALLING GRUP-PC-BIN"
$SUC "DONE!"

$INF "PROBING FOR Bochs"
sudo apt-get -y install bochs &> $WD/error.log || $ERR "ERRORS INSTALLING Bochs"
sudo apt-get -y install bochs-sdl &> $WD/error.log || $ERR "ERRORS INSTALLING Bochs-sdl"
sudo apt-get -y install bochs-x &> $WD/error.log || $ERR "ERRORS INSTALLING Bochs-x"
$SUC "DONE!"

$INF "PROBING FOR kpartx"
sudo apt-get -y install  kpartx &> $WD/error.log || $ERR "ERRORS INSTALLING kpartx"
$SUC "DONE!"

$INF "Attempting to install qemu emulator, Continue? (y/n)"
PROMPTVAR=""
read PROMPTVAR
if [ $PROMPTVAR = 'y' ]; then
  sudo apt-get -y install qemu-system-i386 &> $WD/error.log || $ERR "ERRORS INSTALLING QEMU"
else
  $INF "Skipping QEMU"
fi

$INF "Attempting to install Virtualbox, Continue? (y/n)"
PROMPTVAR=""
read PROMPTVAR
if [ $PROMPTVAR = 'y' ]; then
  sudo apt-get -y install virtualbox &> $WD/error.log || $ERR "ERRORS INSTALLING Virtualbox"
else
  $INF "Skipping Virtualbox"
fi

$INF "DONE INSTALLING BOOT PREREQUISITES!"
