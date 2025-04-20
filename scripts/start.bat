cd ..
if not exist out mkdir out
cd out
if not exist build mkdir build
cd build
if not exist gcc mkdir gcc
cd gcc
if not exist bin mkdir bin
cd bin
cmake -S ../../../../ -B . -G "MinGW Makefiles"
mingw32-make.exe && mingw32-make.exe Shaders
cd ../../../
start ../out/build/gcc/bin/PXT_Engine.exe