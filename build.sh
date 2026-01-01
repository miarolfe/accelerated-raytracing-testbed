#!/bin/bash

cd build
./premake5 gmake

if [[ $# -eq 0 ]]; then
    make config=debug_x64
elif [[ $# -eq 1 && $1 == debug ]]; then
    make config=debug_x64
elif [[ $# -eq 1 && $1 == release ]]; then
    make config=release_x64
elif [[ $# -eq 1 && $1 == test ]]; then
    make config=test_x64
else
    echo "Invalid build arguments, valid options: [none], debug, release, test"
fi


cd ..

if [ -f Makefile ]; then
    rm Makefile
fi
