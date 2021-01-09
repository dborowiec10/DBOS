#!/bin/bash
# param 1 - string
# param 2 - colour RED || GREEN || BLUE
COLOUR_RED=`tput setaf 1`
COLOUR_GREEN=`tput setaf 2`
COLOUR_BLUE=`tput setaf 4`
COLOUR_RESET=`tput sgr0`

case "$2" in
RED)    echo "${COLOUR_RED}$1 ${COLOUR_RESET}"
        printf "\n"
    ;;
GREEN)  echo "${COLOUR_GREEN}$1 ${COLOUR_RESET}"
        printf "\n"
    ;;
BLUE)   echo "${COLOUR_BLUE}$1 ${COLOUR_RESET}"
        printf "\n"
    ;;
REG)    echo "$1 "
        printf "\n"
    ;;
*) exit
   ;;
esac
