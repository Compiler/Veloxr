#!/bin/bash
set -euo pipefail
IFS=$'\n\t'

VERSION=${1:?"missing version arg"}

python -m black conan/conanfile.py

for option in "build_video=True" "build_photo=True"; do
# for option in "build_photo=True"; do
    if wmic os get Caption -value | grep -q "2022"; then
        conan create . conan/conanfile.py \
            --version=$VERSION \
            --options="&:validation_layers=False" \
            --options="&:$option" \
            --remote=topaz-conan \
            -pr:h=profile_win2022_armv8 \
            -pr:b=profile_win2019
        
        conan upload veloxr/$VERSION -r topaz-conan
    else
        conan create conan/conanfile.py  \
            --version=$VERSION \
            --options="&:validation_layers=False" \
            --options="&:$option" \
            --remote=topaz-conan \
            -pr:h=conan/profile_win2019 \
            -pr:b=conan/profile_win2019
        
        conan upload veloxr/$VERSION -r topaz-conan
    fi
done
#conan upload "$PKG_REF" -r topaz-conan
