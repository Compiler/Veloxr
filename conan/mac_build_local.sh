cd ..
conan install conan/conanfile.py -of . \
    --version=0.0.1 \
    --options="&:validation_layers=False" \
    --options="&:build_photo=True" \
    -pr:h=conan/profile_mac_armv8 \
    -pr:b=conan/profile_mac_armv8

cmake -S . -B build -G "Ninja" -DBUILD_TOOLS=True -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=build/Release/generators/conan_toolchain.cmake \
    -DCMAKE_INSTALL_PREFIX=build/veloxr
cmake --build build --target clean
cmake --build build --target install