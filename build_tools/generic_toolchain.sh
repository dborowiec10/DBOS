#!/bin/bash
WD="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

ERR=$(readlink -f error.sh)
SUC=$(readlink -f success.sh)
INF=$(readlink -f info.sh)

VER_BINUTILS="binutils-2.28"
VER_GCC="gcc-7.1.0"

TARGET=i686-elf
PREFIX="$WD/toolchain/cross"

# build initial binutils
pushd builds &> /dev/null || exit
  pushd binutils &> /dev/null || exit
    if [ ! -f .fresh-build-gen ]; then
      rm -rf *
      $WD/sources/$VER_BINUTILS/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --disable-werror &> $WD/error.log || $ERR "ERRORS CONFIGURING GENERIC BINUTILS"
      $INF "CONFIGURED GENERIC BINUTILS"

      make -j8 &> $WD/error.log || $ERR "ERRORS MAKING GENERIC BINUTILS"
      $INF "MADE GENERIC BINUTILS"

      make install &> $WD/error.log || $ERR "ERRORS INSTALLING GENERIC BINUTILS"
      $SUC "SUCCESSFULLY INSTALLED GENERIC BINUTILS"
    else
      $SUC "FRESH BUILD OF $VER_BINUTILS ALREADY PRESENT FOR GENERIC TOOLCHAIN"
    fi
    touch .fresh-build-gen
  popd &> /dev/null || exit
popd &> /dev/null || exit

# build initial gcc
pushd builds &> /dev/null || exit
  pushd gcc &> /dev/null || exit
    if [ ! -f .fresh-build-gen ]; then
      rm -rf *
      $WD/sources/$VER_GCC/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers &> $WD/error.log || $ERR "ERRORS CONFIGURING GENERIC GCC"
      $INF "CONFIGURED GENERIC GCC"

      make -j8 all-gcc &> $WD/error.log || $ERR "ERRORS MAKING GENERIC GCC"
      make -j8 all-target-libgcc &> $WD/error.log || $ERR "ERRORS MAKING GENERIC GCC"
      $INF "MADE GENERIC GCC"

      make install-gcc &> $WD/error.log || $ERR "ERRORS INSTALLING GENERIC GCC"
      make install-target-libgcc &> $WD/error.log || $ERR "ERRORS INSTALLING GENERIC GCC"
      $SUC "SUCCESSFULLY INSTALLED GENERIC GCC"
    else
      $SUC "FRESH BUILD OF $VER_GCC ALREADY PRESENT FOR GENERIC TOOLCHAIN"
    fi
    touch .fresh-build-gen
  popd &> /dev/null || exit
popd &> /dev/null || exit

$SUC "***** ***** SUCCESSFULLY BUILT GENERIC TOOLCHAIN! ***** *****"
