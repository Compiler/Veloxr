#!/bin/bash
set -euo pipefail
IFS=$'\n\t'

# build for both armv8 and x86_64
VERSION=${1:?"missing version arg"}

rm -rf build

python -m black conan/conanfile.py

for option in "build_video=True" "build_photo=True"; do
    conan create conan/conanfile.py  \
        --version=$VERSION \
        --remote=topaz-conan \
        --options="&:validation_layers=False" \
        --options="&:$option" \
        -pr:h=conan/profile_mac_armv8 \
        -pr:b=conan/profile_mac_armv8

    conan create conan/conanfile.py  \
        --version=$VERSION \
        --options="&:validation_layers=False" \
        --options="&:$option" \
        --remote=topaz-conan \
        -pr:h=conan/profile_mac_x86_64 \
        -pr:b=conan/profile_mac_x86_64

    conan upload veloxr/$VERSION -r topaz-conan
done