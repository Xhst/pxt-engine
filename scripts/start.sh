#!/bin/bash
cd ..
mkdir -p out
cd out
mkdir -p build
cd build
mkdir -p gcc
out gcc
mkdir -p bin
cd bin
cmake -S ../../../../ -B .
make && make Shaders
cd ../../../
../out/build/gcc/bin/PXT_Engine