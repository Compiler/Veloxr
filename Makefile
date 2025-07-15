
.PHONY: build

ifeq ($(OS),Windows_NT)

USER := "ljuek"
build:
	glslc.exe src/shaders/passthrough.vert -o spirv/vert.spv 
	glslc.exe src/shaders/passthrough.frag -o spirv/frag.spv
	#if not exist build mkdir build
	#cd build && cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/Users/$(USER)/Code/vcpkg/scripts/buildsystems/vcpkg.cmake && cmake --build . && .\Debug\vulkanrenderer.exe
	./conan/win_local_build.sh
	#./build/vulkanrenderer.exe "C:/Users/$(USER)/Downloads/56000.jpg"
	./build/vulkanrenderer.exe "C:/Users/$(USER)/Downloads/fox.jpg"
else
build:
	glslc.exe src/shaders/passthrough.vert -o spirv/vert.spv 
	glslc.exe src/shaders/passthrough.frag -o spirv/frag.spv
	mkdir -p build
	cd build && cmake .. && cmake --build . && ./vulkanrenderer
endif

run: 
	./build/vulkanrenderer.exe "C:/Users/$(USER)/Downloads/fox.jpg"
runb: 
	./build/vulkanrenderer.exe "C:/Users/$(USER)/Downloads/56000.jpg"
runh: 
	./build/vulkanrenderer.exe "C:/Users/$(USER)/Downloads/fox.jpg"
