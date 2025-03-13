main:
	mkdir -p build
	cd build && cmake .. && cmake --build . && ./vulkanrenderer
