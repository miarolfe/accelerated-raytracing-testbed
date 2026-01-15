#!/bin/bash

cd build
./premake5 gmake

if [[ $# -eq 0 ]]; then
    make config=debug_headless_x64
elif [[ $# -eq 1 && $1 == debug ]]; then
    make config=debug_headless_x64
elif [[ $# -eq 2 && $1 == debug && $2 == headless ]]; then
    make config=debug_headless_x64
elif [[ $# -eq 2 && $1 == debug && $2 == gui ]]; then
    make config=debug_gui_x64
elif [[ $# -eq 1 && $1 == release ]]; then
    make config=release_headless_x64
elif [[ $# -eq 2 && $1 == release && $2 == headless ]]; then
    make config=release_headless_x64
elif [[ $# -eq 2 && $1 == release && $2 == gui ]]; then
    make config=release_gui_x64
elif [[ $# -eq 1 && $1 == test ]]; then
    make config=test_x64
else
    echo "Invalid build arguments. Valid options:"
    echo "  (none)                     -> debug headless"
    echo "  debug [headless|gui]       -> debug build"
    echo "  release [headless|gui]     -> release build"
    echo "  test                       -> test build"
fi



cd ..

if [ -f Makefile ]; then
    rm Makefile
fi
