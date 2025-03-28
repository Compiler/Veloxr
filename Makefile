
.PHONY: build

ifeq ($(OS),Windows_NT)
build:
	if not exist build mkdir build
	cd build && cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/Users/ljuek/Code/vcpkg/scripts/buildsystems/vcpkg.cmake && cmake --build . 
# && .\Debug\vulkanrenderer.exe
else
build:
	mkdir -p build
	cd build && cmake .. && cmake --build . && ./vulkanrenderer
endif
