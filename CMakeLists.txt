cmake_minimum_required(VERSION 3.16)
project(vulkanrenderer)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 20)

set(OpenCV_DIR "C:/Users/ljuek/Code/opencv-install/lib/cmake/opencv4")

#add_library(vulkanrenderer STATIC src/main.cpp src/test.h)
add_executable(vulkanrenderer 
        src/main.cpp 
        src/test.h 
        src/Vertex.h 
        src/device.h src/device.cpp 
        src/OrthographicCamera.h src/OrthographicCamera.cpp
        src/OrthoCam.h src/OrthoCam.cpp
        src/texture.h src/texture.cpp 
        src/TextureTiling.h src/TextureTiling.cpp 
    )

if (MSVC)
    target_compile_options(vulkanrenderer PRIVATE /utf-8)
endif()
target_compile_definitions(vulkanrenderer
    PRIVATE 
        PROJECT_ROOT_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"
)

set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")


find_package(Vulkan REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PUBLIC "src/")

target_include_directories(vulkanrenderer PUBLIC
    "${INCLUDE_DIR}"
)

find_package(OpenCV REQUIRED)
target_include_directories(vulkanrenderer PRIVATE ${OpenCV_INCLUDE_DIRS})

find_package(OpenImageIO REQUIRED)


find_package(glfw3 CONFIG REQUIRED)

target_link_libraries(vulkanrenderer
    PRIVATE
        glfw
        Vulkan::Vulkan
        OpenImageIO::OpenImageIO
        ${OpenCV_LIBS}
)
