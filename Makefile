
.PHONY: build

ifeq ($(OS),Windows_NT)
build:
	glslc.exe src/shaders/passthrough.vert -o spirv/vert.spv 
	glslc.exe src/shaders/passthrough.frag -o spirv/frag.spv
	#if not exist build mkdir build
	#cd build && cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/Users/ljuek/Code/vcpkg/scripts/buildsystems/vcpkg.cmake && cmake --build . && .\Debug\vulkanrenderer.exe
	./conan/win_local_build.sh
	./build/vulkanrenderer.exe "C:/Users/ljuek/Downloads/test.png"
else
build:
	glslc.exe src/shaders/passthrough.vert -o spirv/vert.spv 
	glslc.exe src/shaders/passthrough.frag -o spirv/frag.spv
	mkdir -p build
	cd build && cmake .. && cmake --build . && ./vulkanrenderer
endif

run: 
	./build/vulkanrenderer.exe "C:/Users/ljuek/Downloads/test.png"
