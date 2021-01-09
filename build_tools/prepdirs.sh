#!/bin/bash
WD="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

ERR=$(readlink -f error.sh)
SUC=$(readlink -f success.sh)
INF=$(readlink -f info.sh)

DIRS=(binutils binutils-host gcc gcc-host autotools)

if [ ! -d "builds" ]; then
  mkdir builds
fi

if [ ! -d "toolchain" ]; then
  mkdir toolchain
fi

pushd builds &> /dev/null || exit
  for dir in ${DIRS[@]}; do
    if [ ! -d $dir ]; then
      mkdir $dir
    fi
    if [ ! -f ./$dir/.fresh-build-gen ]; then
      rm -rf ./$dir/*
    fi
  done
popd &> /dev/null || exit

pushd toolchain &> /dev/null || exit
  if [ ! -d "cross" ]; then
    mkdir -p cross
  fi

  if [ ! -d "host" ]; then
    mkdir -p host
  fi

  if [ ! -d "autotools" ]; then
    mkdir -p autotools
  fi
popd &> /dev/null || exit

$INF "PREPARED BUILD SPACE"
