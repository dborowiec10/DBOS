#!/bin/bash
WD="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

ERR=$(readlink -f error.sh)
SUC=$(readlink -f success.sh)
INF=$(readlink -f info.sh)

PROMPTVAR=""
AUTOMATE=0

chmod +x $WD/acquire.sh
chmod +x $WD/untar.sh
chmod +x $WD/prepdirs.sh
chmod +x $WD/install_prerequisites.sh
chmod +x $WD/generic_toolchain.sh
chmod +x $WD/autotools.sh
chmod +x $WD/host_toolchain.sh
chmod +x $WD/error.sh
chmod +x $WD/info.sh
chmod +x $WD/out.sh
chmod +x $WD/success.sh

$INF "Welcome to DBOS toolchain build script."
$INF "Build process can be fully automatic or prompted at each stage."
$INF "Would you like to make the build process fully automatic? (y/n)"
PROMPTVAR=""
read PROMPTVAR
if [ $PROMPTVAR = 'y' ]; then
  AUTOMATE=1
fi

$INF "DOWNLOADING SOURCES"
if [ $AUTOMATE = 0 ]; then
  $INF "Continue? (y/n)"
  read PROMPTVAR
  if [ $PROMPTVAR = 'y' ]; then
    . $WD/acquire.sh
  else
    $INF "Exiting build process..."
    exit 0
  fi
else
  . $WD/acquire.sh
fi

$INF "EXTRACTING SOURCES"
if [ $AUTOMATE = 0 ]; then
  $INF "Continue? (y/n)"
  read PROMPTVAR
  if [ $PROMPTVAR = 'y' ]; then
    . $WD/untar.sh
    . $WD/prepdirs.sh
  else
    $INF "Exiting build process..."
    exit 0
  fi
else
  . $WD/untar.sh
  . $WD/prepdirs.sh
fi

$INF "INSTALLING BUILD PREREQUISITES"
if [ $AUTOMATE = 0 ]; then
  $INF "Continue? (y/n)"
  read PROMPTVAR
  if [ $PROMPTVAR = 'y' ]; then
    . $WD/install_prerequisites.sh
  else
    $INF "Exiting build process..."
    exit 0
  fi
else
  . $WD/install_prerequisites.sh
fi

$INF "INSTALLING GENERIC TOOLCHAIN"
if [ $AUTOMATE = 0 ]; then
  $INF "Continue? (y/n)"
  read PROMPTVAR
  if [ $PROMPTVAR = 'y' ]; then
    . $WD/generic_toolchain.sh
  else
    $INF "Exiting build process..."
    exit 0
  fi
else
  . $WD/generic_toolchain.sh
fi

$INF "INSTALLING AUTOTOOLS"
if [ $AUTOMATE = 0 ]; then
  $INF "Continue? (y/n)"
  read PROMPTVAR
  if [ $PROMPTVAR = 'y' ]; then
    . $WD/autotools.sh
  else
    $INF "Exiting build process..."
    exit 0
  fi
else
  . $WD/autotools.sh
fi

$INF "INSTALLING HOST TOOLCHAIN"
if [ $AUTOMATE = 0 ]; then
  $INF "Continue? (y/n)"
  read PROMPTVAR
  if [ $PROMPTVAR = 'y' ]; then
    . $WD/host_toolchain.sh
  else
    $INF "Exiting build process..."
    exit 0
  fi
else
  . $WD/host_toolchain.sh
fi
