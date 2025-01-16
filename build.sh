#! /bin/bash

# setup build dir
rm -rf build || true
mkdir -p build

# build
cd build
cmake ..
cmake --build .
cd ..
