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

if [ ! -d "sources" ]; then
  mkdir sources
fi

# first lets download all sources into sources
# if they are not already there
# GCC
if [ ! -f "$WD/sources/$VER_GCC.tar.gz" ]; then
  $INF "DOWNLOADING $VER_GCC"
  wget -O $WD/sources/$VER_GCC.tar.gz -o $WD/error.log "https://ftp.gnu.org/gnu/gcc/$VER_GCC/$VER_GCC.tar.gz" || $ERR "ERROR DONWLOADING GCC"
  $SUC "DOWNLOADED GCC"
else
  $SUC "CORRECT VERSION OF GCC PRESENT!"
fi

# Binutils
if [ ! -f "$WD/sources/$VER_BINUTILS.tar.gz" ]; then
  $INF "DONWLOADING $VER_BINUTILS"
  wget -O $WD/sources/$VER_BINUTILS.tar.gz -o $WD/error.log "https://ftp.gnu.org/gnu/binutils/$VER_BINUTILS.tar.gz" || $ERR "ERROR DONWLOADING BINUTILS"
  $SUC "DOWNLOADED BINUTILS"
else
  $SUC "CORRECT VERSION OF BINUTILS PRESENT!"
fi

# Autoconfs
for i in "${SOURCES_AUTOCONF[@]}"
do
  if [ ! -f "$WD/sources/$i.tar.gz" ]; then
    $INF "DOWNLOADING $i"
    wget -O $WD/sources/$i.tar.gz -o $WD/error.log "https://ftp.gnu.org/gnu/autoconf/$i.tar.gz" || $ERR "ERROR DONWLOADING $i"
    $SUC "DOWNLOADED $i"
  else
    $SUC "CORRECT VERSION OF $i PRESENT!"
  fi
done

# Automakes
for i in "${SOURCES_AUTOMAKE[@]}"
do
  if [ ! -f "$WD/sources/$i.tar.gz" ]; then
    $INF "DOWNLOADING $i"
    wget -O $WD/sources/$i.tar.gz -o $WD/error.log "https://ftp.gnu.org/gnu/automake/$i.tar.gz" || $ERR "ERROR DONWLOADING $i"
    $SUC "DOWNLOADED $i"
  else
    $SUC "CORRECT VERSION OF $i PRESENT!"
  fi
done
