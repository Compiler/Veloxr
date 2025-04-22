#!/bin/bash
set -euo pipefail
IFS=$'\n\t'

#conan remove "*" --confirm

cd ..
conan install deploy/conanfile.py -of . \
    --version=2.3.0 \
    -pr:h=deploy/profile_win2019 \
    -pr:b=deploy/profile_win2019

cmake -S . -B build -G "Ninja" -DBUILD_TOOLS=True -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=build/generators/conan_toolchain.cmake
cmake --build build --target clean
cmake --build build
