#!/bin/bash
set -euo pipefail
IFS=$'\n\t'

#conan remove "*" --confirm

rm -rf build

# Change build_video to build_photo to build photo only
conan install conan/conanfile.py -of . \
    --version=2.3.0 \
    --options="&:validation_layers=False" \
    --options="&:build_video=True" \
    -pr:h=conan/profile_win2022 \
    -pr:b=conan/profile_win2022

cmake -S . -B build -G "Ninja" -DBUILD_TOOLS=True -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=build/generators/conan_toolchain.cmake
cmake --build build --target clean
cmake --build build
