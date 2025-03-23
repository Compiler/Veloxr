cmake_minimum_required(VERSION 3.16)
project(vulkanrenderer)


set(CMAKE_CXX_STANDARD 20)

add_executable(vulkanrenderer src/main.cpp)

target_compile_definitions(vulkanrenderer
    PRIVATE 
        PROJECT_ROOT_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"
)

set(MY_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")


find_package(Vulkan REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan)

target_include_directories(vulkanrenderer PUBLIC
    "${MY_INCLUDE_DIR}"
)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} glfw)
