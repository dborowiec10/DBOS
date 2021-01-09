#!/bin/bash
# param 1 = string
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
OUT=$(readlink -f $DIR/out.sh)
. $OUT "$1" RED
cat $DIR/error.log
exit
