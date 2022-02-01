#!/bin/bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -D CMAKE_C_COMPILER=arm-opi-linux-gnueabihf-gcc -D CMAKE_CXX_COMPILER=arm-opi-linux-gnueabihf-gcc -G Ninja ..
