cd ..
conan install conan/conanfile.py -of . \
    --version=0.0.1 \
    -pr:h=conan/profile_mac_armv8 \
    -pr:b=conan/profile_mac_armv8

cmake -S . -B build -G "Ninja" -DBUILD_TOOLS=True -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=build/Release/generators/conan_toolchain.cmake
cmake --build build --target clean
cmake --build build