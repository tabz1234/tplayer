#!/bin/bash

rm -rf build/

mkdir build

mkdir build/debug
mkdir build/release
mkdir build/minSizeRel
mkdir build/relWithDebInfo

cmake -G="Ninja" -DCMAKE_BUILD_TYPE=Release  -S . -B ./build/release
cmake -G="Ninja" -DCMAKE_BUILD_TYPE=Debug  -S . -B ./build/debug
cmake -G="Ninja" -DCMAKE_BUILD_TYPE=MinSizeRel  -S . -B ./build/minSizeRel
cmake -G="Ninja" -DCMAKE_BUILD_TYPE=RelWithDebInfo  -S . -B ./build/relWithDebInfo
