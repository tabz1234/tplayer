#!/bin/bash
mkdir build
mkdir build/debug
mkdir build/release

cmake  -DCMAKE_BUILD_TYPE=release  -S . -B ./build/release
cmake  -DCMAKE_BUILD_TYPE=debug  -S . -B ./build/debug
