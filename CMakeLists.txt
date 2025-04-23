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
find_package(glm REQUIRED)

set(LIB_SOURCES
    src/Vertex.h 
    src/device.h src/device.cpp 
    src/OrthographicCamera.h src/OrthographicCamera.cpp
    src/OrthoCam.h src/OrthoCam.cpp
    src/texture.h src/texture.cpp 
    src/TextureTiling.h src/TextureTiling.cpp
)

# Create the library
add_library(veloxr_lib STATIC ${LIB_SOURCES})

if (MSVC)
    target_compile_options(veloxr_lib PRIVATE /utf-8)
endif()


set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_include_directories(veloxr_lib 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${INCLUDE_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE 
        ${Vulkan_INCLUDE_DIRS}
        ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(veloxr_lib
    PUBLIC
        glfw
        Vulkan::Vulkan
        OpenImageIO::OpenImageIO
        opencv::opencv
        glm::glm
)

add_executable(vulkanrenderer src/main.cpp)

if (MSVC)
    target_compile_options(vulkanrenderer PRIVATE /utf-8)
endif()

target_link_libraries(vulkanrenderer
    PRIVATE
        veloxr_lib
)

# Installation setup
install(TARGETS vulkanrenderer
    RUNTIME DESTINATION bin
)

install(TARGETS veloxr_lib
    EXPORT veloxrTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

# Export the targets
install(EXPORT veloxrTargets
    FILE veloxrTargets.cmake
    NAMESPACE veloxr::
    DESTINATION lib/cmake/veloxr
)

# Dont need anything in install after using glm from conan
# Install header files (preserve folder structure)
# install(DIRECTORY include/
#     DESTINATION include/veloxr
#     FILES_MATCHING PATTERN "*.h"
# )

# Install source header files (that are part of the public API)
install(DIRECTORY src/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

# Install shader files
install(DIRECTORY src/shaders/
    DESTINATION shaders
    FILES_MATCHING PATTERN "*.vert" PATTERN "*.frag" PATTERN "*.comp" PATTERN "*.geom" PATTERN "*.tesc" PATTERN "*.tese" PATTERN "*.spv"
)

# Simple install of SPIRV files from project directory
install(DIRECTORY spirv/
    DESTINATION spirv
    FILES_MATCHING PATTERN "*.spv"
)
