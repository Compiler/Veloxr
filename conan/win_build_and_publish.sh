#!/bin/bash
set -euo pipefail
IFS=$'\n\t'

VERSION=${1:?"missing version arg"}

# if wmic os get Caption -value | grep -q "2022"; then
#     conan create . conan/conanfile.py \
#         --version=$VERSION \
#         --options="&:validation_layers=False" \
#         --remote=topaz-conan \
#         -pr:h=profile_win2022_armv8 \
#         -pr:b=profile_win2022
    
#     conan upload veloxr/$VERSION -r topaz-conan
# else
#     conan create conan/conanfile.py  \
#         --version=$VERSION \
#         --options="&:validation_layers=False" \
#         --remote=topaz-conan \
#         -pr:h=conan/profile_win2022 \
#         -pr:b=conan/profile_win2022
    
#     conan upload veloxr/$VERSION -r topaz-conan
# fi

conan create conan/conanfile.py  \
    --version=$VERSION \
    --options="&:validation_layers=False" \
    --remote=topaz-conan \
    -pr:h=conan/profile_win2022 \
    -pr:b=conan/profile_win2022

conan upload veloxr/$VERSION -r topaz-conan