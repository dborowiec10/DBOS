#!/bin/bash
WD="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

ERR=$(readlink -f error.sh)
SUC=$(readlink -f success.sh)
INF=$(readlink -f info.sh)

build_single_autotool () {
  pushd toolchain &> /dev/null || exit
    pushd autotools &> /dev/null || exit
      if [ ! -d $1 ]; then
        mkdir $1
      fi
      rm -rf ./$1/*
    popd &> /dev/null || exit
  popd &> /dev/null || exit
  pushd builds &> /dev/null || exit
    pushd autotools &> /dev/null || exit
      if [ ! -d $1 ]; then
        mkdir $1
      fi
      rm -rf ./$1/*
      pushd $1 &> /dev/null || exit
        $WD/sources/$1/configure --prefix=$WD/toolchain/autotools/$1 &> $WD/error.log || $ERR "ERRORS CONFIGURING $1"
        $INF "CONFIGURED $1"
        make -j8 &> $WD/error.log || $ERR "ERRORS MAKING $1"
        $INF "MADE $1"
        make install &> $WD/error.log || $ERR "ERRORS INSTALLING $1"
        $SUC "SUCCESSFULLY INSTALLED $1"
      popd &> /dev/null || exit
    popd &> /dev/null || exit
  popd &> /dev/null || exit
}

build_combined_autotool () {
  pushd toolchain &> /dev/null || exit
    pushd autotools &> /dev/null || exit
      if [ ! -d $1_$2 ]; then
        mkdir $1_$2
      fi
      rm -rf ./$1_$2/*
    popd &> /dev/null || exit
  popd &> /dev/null || exit
  pushd builds &> /dev/null || exit
    pushd autotools &> /dev/null || exit
      if [ ! -d $1_$2 ]; then
        mkdir $1_$2
      fi
      rm -rf ./$1_$2/*
      pushd $1_$2 &> /dev/null || exit
        export PATH="$WD/toolchain/autotools/autoconf-2.65/bin:$PATH"
        export PATH="$WD/toolchain/autotools/autoconf-2.64/bin:$PATH"
        $WD/sources/$1/configure --prefix=$WD/toolchain/autotools/$1_$2 &> $WD/error.log || $ERR "ERRORS CONFIGURING $1"
        $INF "CONFIGURED $1"
        make V=1 VERBOSE=1 &> $WD/error.log || $ERR "ERRORS MAKING $1"
        $INF "MADE $1"
        make install &> $WD/error.log || $ERR "ERRORS INSTALLING $1"
        $SUC "SUCCESSFULLY INSTALLED $1"
        $WD/sources/$2/configure --prefix=$WD/toolchain/autotools/$1_$2 &> $WD/error.log || $ERR "ERRORS CONFIGURING $2"
        $INF "CONFIGURED $2"
        make V=1 VERBOSE=1 &> $WD/error.log || $ERR "ERRORS MAKING $2"
        $INF "MADE $2"
        make install &> $WD/error.log || $ERR "ERRORS INSTALLING $2"
        $SUC "SUCCESSFULLY INSTALLED $2"
      popd &> /dev/null || exit
    popd &> /dev/null || exit
  popd &> /dev/null || exit
}

build_single_autotool "autoconf-2.64"
build_single_autotool "autoconf-2.65"
build_combined_autotool "automake-1.12" "autoconf-2.65"
build_combined_autotool "automake-1.11.6" "autoconf-2.64"
