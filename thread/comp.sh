#!/bin/bash

if [ $# -eq 0 ]; then
    echo "Need a source file!"
    exit 1
fi

gcc -g -std=c99 -Wall -Wextra "$1" -I./

if  [ $? -ne 0 ]; then
    echo "Compilation failed!"
    exit 1
fi
