#!/bin/bash

rm -rf build/

mkdir build

cmake -G="Ninja" -DCMAKE_BUILD_TYPE=Release  -S . -B ./build/release
cmake -G="Ninja" -DCMAKE_BUILD_TYPE=MinSizeRel  -S . -B ./build/minSizeRel

cmake -G="Ninja" -DCMAKE_BUILD_TYPE=Debug  -S . -B ./build/debug_0
cmake -G="Ninja" -DCMAKE_BUILD_TYPE=Debug  -S . -B ./build/debug_1
cmake -G="Ninja" -DCMAKE_BUILD_TYPE=Debug  -S . -B ./build/debug_2
cmake -G="Ninja" -DCMAKE_BUILD_TYPE=Debug  -S . -B ./build/debug_3
