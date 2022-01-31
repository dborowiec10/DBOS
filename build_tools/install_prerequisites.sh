#!/bin/bash
WD="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

ERR=$(readlink -f error.sh)
SUC=$(readlink -f success.sh)
INF=$(readlink -f info.sh)

$INF "PROBING FOR GNU GMP..."
sudo apt-get -y install libgmp3-dev &> $WD/error.log || $ERR "ERRORS INSTALLING GNU GMP"
$SUC "DONE!"

$INF "PROBING FOR GNU MPFR..."
sudo apt-get -y install libmpfr-dev &> $WD/error.log || $ERR "ERRORS INSTALLING GNU MPFR"
$SUC "DONE!"

$INF "PROBING FOR GNU MPC..."
sudo apt-get -y install libmpc-dev &> $WD/error.log || $ERR "ERRORS INSTALLING GNU MPC"
$SUC "DONE!"

$INF "PROBING FOR GNU Texinfo"
sudo apt-get -y install texinfo-doc-nonfree &> $WD/error.log || $ERR "ERRORS INSTALLING GNU Texinfo"
$SUC "DONE!"

$INF "PROBING FOR Makeinfo"
sudo apt-get -y install texinfo &> $WD/error.log || $ERR "ERRORS INSTALLING GNU Texinfo"
$SUC "DONE!"

$INF "PROBING FOR GNU Make"
sudo apt-get -y install make &> $WD/error.log || $ERR "ERRORS INSTALLING GNU Make"
$SUC "DONE!"

$INF "PROBING FOR GNU M4"
sudo apt-get -y install m4 &> $WD/error.log || $ERR "ERRORS INSTALLING GNU M4"
$SUC "DONE!"

$INF "PROBING FOR YASM..."
sudo apt-get -y install yasm &> $WD/error.log || $ERR "ERRORS INSTALLING YASM"
$SUC "DONE!"

$INF "DONE INSTALLING PREREQUISITES!"
