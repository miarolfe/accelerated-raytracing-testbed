#!/bin/bash

cd build
./premake5 gmake
cd ..

if [ $# -eq 0 ]; then
    # No parameters supplied, assume Debug mode build
    make config=Debug_x64
elif [ $# -eq 1 && $1 == "debug" ]; then
    make config=Debug_x64
elif [ $# -eq 1 && $1 == "release" ]; then
    make config=Release_x64
else
    echo "Invalid build arguments, valid options: [none], debug_x64, release_x64"
fi
