cmake_minimum_required(VERSION 3.16)
project(vulkanrenderer)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 20)

find_package(OpenCV REQUIRED)
find_package(OpenImageIO REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glm REQUIRED)
find_package(MoltenVK CONFIG REQUIRED)

option(VALIDATION_LAYERS "Enable validation layers" OFF)

set(LIB_SOURCES
    src/Vertex.h 
    src/renderer.h src/renderer.cpp
    src/device.h src/device.cpp 
    src/OrthographicCamera.h src/OrthographicCamera.cpp
    src/OrthoCam.h src/OrthoCam.cpp
    src/texture.h src/texture.cpp 
    src/TextureTiling.h src/TextureTiling.cpp
    src/MetalSurfaceHelper.h src/MetalSurfaceHelper.mm
)

# Create the library
add_library(veloxr_lib SHARED ${LIB_SOURCES})

if (MSVC)
    target_compile_options(veloxr_lib PRIVATE /utf-8)
endif()

add_compile_definitions(VALIDATION_LAYERS=$<BOOL:${VALIDATION_LAYERS}>)
message(STATUS "Validation layers are ${VALIDATION_LAYERS}")


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
        moltenvk::moltenvk
)

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    # Set the install name of the library
    set_target_properties(veloxr_lib PROPERTIES
        INSTALL_NAME_DIR "@rpath"
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "@executable_path/../Frameworks"
    )
    
    target_link_libraries(veloxr_lib
        PUBLIC
            "-framework QuartzCore"
            "-framework Cocoa"
    )
endif()

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

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # # Add debug info for release builds if needed
    # target_compile_options(vulkanrenderer PRIVATE "$<$<CONFIG:Release>:/Zi>")
    # target_link_options(vulkanrenderer PRIVATE "$<$<CONFIG:Release>:/DEBUG;/INCREMENTAL:NO;/OPT:REF;/OPT:ICF>")

    # Copy runtime DLLs that vulkanrenderer depends on
    add_custom_command(TARGET vulkanrenderer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_RUNTIME_DLLS:vulkanrenderer>" "$<TARGET_FILE_DIR:vulkanrenderer>"
        COMMAND_EXPAND_LISTS)

    list(TRANSFORM CMAKE_MODULE_PATH APPEND "/bin/*")
    message(STATUS "***CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")
    file(GLOB DYNAMIC_LIBS ${CMAKE_MODULE_PATH})
    foreach(DLL_FILE ${DYNAMIC_LIBS})
        add_custom_command(TARGET vulkanrenderer POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "${DLL_FILE}" "$<TARGET_FILE_DIR:vulkanrenderer>")
    endforeach()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_compile_options(vulkanrenderer PRIVATE "$<$<CONFIG:Release>:-g>")
    set_target_properties(vulkanrenderer PROPERTIES BUILD_RPATH "@executable_path")
    # Fix rpath issues with macOS libraries
    add_custom_command(TARGET vulkanrenderer POST_BUILD
        COMMAND ${CMAKE_COMMAND} "-DTARGET_FILE=$<TARGET_FILE:vulkanrenderer>" -P "${CMAKE_SOURCE_DIR}/conan/fix_mac_libs.cmake")
    
    # Find and copy all dylib files from Conan packages
    foreach(DIR_PATH ${CMAKE_MODULE_PATH})
        file(GLOB LIB_FILES "${DIR_PATH}/lib/*.dylib")
        foreach(LIB ${LIB_FILES})
            add_custom_command(TARGET vulkanrenderer POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different 
                "${LIB}" "$<TARGET_FILE_DIR:vulkanrenderer>"
                COMMENT "Copying ${LIB} to build directory")
        endforeach()
    endforeach()
endif()
