#!/bin/bash
WD="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

ERR=$(readlink -f error.sh)
SUC=$(readlink -f success.sh)
INF=$(readlink -f info.sh)

VER_BINUTILS="binutils-2.28"
VER_GCC="gcc-7.1.0"

VER_AUTOMAKE1="automake-1.11.6"
VER_AUTOMAKE2="automake-1.12"
VER_AUTOCONF1="autoconf-2.64"
VER_AUTOCONF2="autoconf-2.65"

SOURCES_AUTOCONF=($VER_AUTOCONF1 $VER_AUTOCONF2)
SOURCES_AUTOMAKE=($VER_AUTOMAKE1 $VER_AUTOMAKE2)

# untar sources
pushd sources &> /dev/null || exit
  if [ ! -f "$VER_GCC/.fresh" ]; then
    rm -rf $VER_GCC
    tar -xvzf $VER_GCC.tar.gz &> $WD/error.log || $ERR "ERROR UNPACKING GCC"
    touch $VER_GCC/.fresh
    $SUC "UNPACKED GCC!"
  else
    $SUC "FRESH SOURCES FOR GCC PRESENT"
  fi

  if [ ! -f "$VER_BINUTILS/.fresh" ]; then
    rm -rf $VER_BINUTILS
    tar -xvzf $VER_BINUTILS.tar.gz &> $WD/error.log || $ERR "ERROR UNPACKING BINUTILS"
    touch $VER_BINUTILS/.fresh
    $SUC "UNPACKED BINUTILS!"
  else
    $SUC "FRESH SOURCES FOR BINUTILS PRESENT"
  fi

  for i in "${SOURCES_AUTOMAKE[@]}"
  do
    if [ ! -f "$i/.fresh" ]; then
      rm -rf $i
      tar -xvzf $i.tar.gz &> $WD/error.log || $ERR "ERROR UNPACKING $i"
      touch $i/.fresh
      $SUC "UNPACKED $i!"
    else
      $SUC "FRESH SOURCES FOR $i PRESENT"
    fi
  done

  for j in "${SOURCES_AUTOCONF[@]}"
  do
    if [ ! -f "$j/.fresh" ]; then
      rm -rf $j
      tar -xvzf $j.tar.gz &> $WD/error.log || $ERR "ERROR UNPACKING $j"
      touch $j/.fresh
      $SUC "UNPACKED $j!"
    else
      $SUC "FRESH SOURCES FOR $j PRESENT"
    fi
  done

popd &> /dev/null || exit
