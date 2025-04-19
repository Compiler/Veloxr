cmake_minimum_required(VERSION 3.16)
project(vulkanrenderer)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 20)

# set(OpenCV_DIR "C:/Users/ljuek/Code/opencv-install/lib/cmake/opencv4")

find_package(OpenCV REQUIRED)
find_package(OpenImageIO REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(Vulkan REQUIRED)

set(LIB_SOURCES
    src/Vertex.h 
    src/device.h src/device.cpp 
    src/OrthographicCamera.h src/OrthographicCamera.cpp
    src/OrthoCam.h src/OrthoCam.cpp
    src/texture.h src/texture.cpp 
    src/TextureTiling.h src/TextureTiling.cpp
)

# First, create the library
add_library(veloxr_lib STATIC ${LIB_SOURCES})

if (MSVC)
    target_compile_options(veloxr_lib PRIVATE /utf-8)
endif()

target_compile_definitions(veloxr_lib
    PRIVATE 
        PROJECT_ROOT_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"
)

set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_include_directories(veloxr_lib PUBLIC ${Vulkan_INCLUDE_DIRS})
target_include_directories(veloxr_lib PUBLIC "src/")
target_include_directories(veloxr_lib PUBLIC "${INCLUDE_DIR}")
target_include_directories(veloxr_lib PRIVATE ${OpenCV_INCLUDE_DIRS})

target_link_libraries(veloxr_lib
    PUBLIC
        glfw
        Vulkan::Vulkan
        OpenImageIO::OpenImageIO
        opencv::opencv
)

add_executable(vulkanrenderer src/main.cpp src/test.h)

if (MSVC)
    target_compile_options(vulkanrenderer PRIVATE /utf-8)
endif()

target_link_libraries(vulkanrenderer
    PRIVATE
        veloxr_lib
)

# Install the executable to bin directory and the library to lib directory
install(TARGETS vulkanrenderer
    RUNTIME DESTINATION bin
)

install(TARGETS veloxr_lib
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Install header files
install(DIRECTORY include/
    DESTINATION include/veloxr
    FILES_MATCHING PATTERN "*.h"
)

# Install source header files if needed
install(DIRECTORY src/
    DESTINATION include/veloxr
    FILES_MATCHING PATTERN "*.h"
)