#!/bin/sh

set -xe

clear
rm "$1"
gcc "$1".c -Wall -Wpedantic -Wextra -o "$1"
./"$1"