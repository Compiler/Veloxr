
.PHONY: build

ifeq ($(OS),Windows_NT)
build:
	glslc.exe src/shaders/passthrough.vert -o spirv/vert.spv 
	glslc.exe src/shaders/passthrough.frag -o spirv/frag.spv
	#if not exist build mkdir build
	#cd build && cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/Users/ljuek/Code/vcpkg/scripts/buildsystems/vcpkg.cmake && cmake --build . && .\Debug\vulkanrenderer.exe
	./conan/win_local_build.sh
	#./build/vulkanrenderer.exe "C:/Users/ljuek/Downloads/56000.jpg"
	./build/vulkanrenderer.exe "C:/Users/ljuek/Downloads/fox.jpg"
else
build:
	# Check if glslc exists, otherwise use a different approach
	if command -v glslc >/dev/null 2>&1; then \
		glslc src/shaders/passthrough.vert -o spirv/vert.spv; \
		glslc src/shaders/passthrough.frag -o spirv/frag.spv; \
		glslc src/shaders/passthrough_mac.frag -o spirv/frag_mac.spv; \
	else \
		echo "glslc not found, shaders will be compiled during build"; \
	fi
	mkdir -p build
	cd build && cmake .. && cmake --build . && ./vulkanrenderer
endif

run: 
	./build/vulkanrenderer.exe "C:/Users/ljuek/Downloads/fox.jpg"
runb: 
	./build/vulkanrenderer.exe "C:/Users/ljuek/Downloads/56000.jpg"
