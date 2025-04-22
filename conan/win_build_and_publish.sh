#!/bin/bash
set -euo pipefail
IFS=$'\n\t'

VERSION=${1:?"missing version arg"}

if wmic os get Caption -value | grep -q "2022"; then
    conan create . conan/conanfile.py \
        --update \
        --version=$VERSION \
        --remote=topaz-conan \
        -pr:h=profile_win2022_armv8 \
        -pr:b=profile_win2019
else
    conan create conan/conanfile.py  \
        --update \
        --version=$VERSION \
        --remote=topaz-conan \
        -pr:h=conan/profile_win2019 \
        -pr:b=conan/profile_win2019
fi

conan upload veloxr/$VERSION -r topaz-conan

#conan upload "$PKG_REF" -r topaz-conan
