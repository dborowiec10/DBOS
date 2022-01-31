#!/bin/bash
WD="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

ERR=$(readlink -f error.sh)
SUC=$(readlink -f success.sh)
INF=$(readlink -f info.sh)

PREFIX=$WD/toolchain/host

TARGET=i686-dbos

BINUTILS_VER="binutils-2.28"

GCC_VER="gcc-7.1.0"

SYSROOT=$(readlink -f ../sysroot)

VER_AUTOTOOL1="automake-1.11.6_autoconf-2.64"

if [[ (! -f builds/binutils-host/.all-good-binutils) && (! -f builds/gcc-host/.all-good-gcc ) ]]; then
  pushd sources &> /dev/null || exit
    # prep directories for sources
    if [ -d host_${BINUTILS_VER} ]; then
      rm -rf host_${BINUTILS_VER}
    fi
    mkdir host_${BINUTILS_VER}

    if [ -d host_${GCC_VER} ]; then
      rm -rf host_${GCC_VER}
    fi
    mkdir host_${GCC_VER}

    # untar binutils to a separate directory
    if [ ! -f host_$BINUTILS_VER/.fresh_host ]; then
      tar -xvzf $BINUTILS_VER.tar.gz -C host_$BINUTILS_VER --strip-components 1 &> $WD/error.log || $ERR "ERRORS UNPACKING BINUTILS"
      touch host_$BINUTILS_VER/.fresh_host
      $SUC "UNPACKED BINUTILS!"
    else
      $SUC "FRESH SOURCES FOR HOST BINUTILS READY"
    fi

    # untar gcc to a separate directory
    if [ ! -f host_$GCC_VER/.fresh_host ]; then
      tar -xvzf $GCC_VER.tar.gz -C host_$GCC_VER --strip-components 1 &> $WD/error.log || $ERR "ERRORS UNPACKING GCC"
      touch host_$GCC_VER/.fresh_host
      $SUC "UNPACKED GCC!"
    else
      $SUC "FRESH SOURCES FOR HOST GCC READY"
    fi

    # patch binutils and automake ld
    pushd "host_$BINUTILS_VER" &> /dev/null || exit
      patch -p1 < $WD/patches/$BINUTILS_VER.patch &> $WD/error.log || $ERR "ERRORS PATCHING BINUTILS SOURCES"
      $INF "PATCHED BINUTILS SOURCES"

      pushd ld &> /dev/null || exit
        env PATH="$WD/toolchain/autotools/$VER_AUTOTOOL1/bin:$PATH" automake
      popd &> /dev/null || exit

    popd &> /dev/null || exit

    # patch gcc and autoconf libstdc++-v3
    pushd "host_$GCC_VER" &> /dev/null || exit
      patch -p1 < $WD/patches/$GCC_VER.first.patch &> $WD/error.log || $ERR "ERRORS PATCHING GCC SOURCES 1"
      $INF "PATCHED GCC SOURCES 1"

      pushd libstdc++-v3 &> /dev/null || exit
        env PATH="$WD/toolchain/autotools/$VER_AUTOTOOL1/bin:$PATH" autoconf
      popd &> /dev/null || exit

      patch -p1 < $WD/patches/$GCC_VER.second.patch &> $WD/error.log || $ERR "ERRORS PATCHING GCC SOURCES 1"
      $INF "PATCHED GCC SOURCES 2"
    popd &> /dev/null || exit

  popd &> /dev/null || exit

  # build binutils
  pushd builds > /dev/null || exit
    if [ -d binutils-host ]; then
      rm -rf binutils-host
    fi
    mkdir binutils-host
    pushd binutils-host > /dev/null || exit
    $WD/sources/host_${BINUTILS_VER}/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot=$SYSROOT --disable-nls --disable-werror &> $WD/error.log || $ERR "ERRORS CONFIGURING HOST BINUTILS"
    $INF "CONFIGURED HOST BINUTILS"
    make -j8 &> $WD/error.log || $ERR "ERRORS MAKING HOST BINUTILS"
    $INF "MADE HOST BINUTILS"
    make install &> $WD/error.log || $ERR "ERRORS INSTALLING HOST BINUTILS"
    $SUC "***** ***** SUCCESSFULLY INSTALLED HOST BINUTILS! ***** *****"
    popd > /dev/null || exit
  popd > /dev/null || exit

  # build gcc
  pushd builds > /dev/null || exit
    if [ -d gcc-host ]; then
      rm -rf gcc-host
    fi
    mkdir gcc-host
    pushd gcc-host > /dev/null || exit
    $WD/sources/host_${GCC_VER}/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot=$SYSROOT --enable-languages=c &> $WD/error.log || $ERR "ERRORS CONFIGURING HOST GCC"
    $INF "CONFIGURED HOST GCC"
    make -j8 all-gcc &> $WD/error.log || $ERR "ERRORS MAKING HOST GCC"
    make -j8 all-target-libgcc &> $WD/error.log || $ERR "ERRORS MAKING HOST GCC"

    $INF "MADE HOST GCC"
    make install-gcc install-target-libgcc &> $WD/error.log || $ERR "ERRORS INSTALLING HOST GCC"
    $SUC "***** ***** SUCCESSFULLY INSTALLED HOST GCC! ***** *****"
    popd > /dev/null || exit
  popd > /dev/null || exit

  touch $WD/builds/gcc-host/.all-good-gcc
  touch $WD/builds/binutils-host/.all-good-binutils
else
  $INF "HOST GCC ALREADY INSTALLED!"
  $INF "HOST BINUTILS ALREADY INSTALLED"
fi
