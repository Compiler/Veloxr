cmake_minimum_required(VERSION 3.16)
project(vulkanrenderer)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)

# Add Objective-C++ support for macOS
if(APPLE)
    set(CMAKE_OBJCXX_STANDARD 20)
    set(CMAKE_OBJCXX_STANDARD_REQUIRED ON)
endif()

find_package(OpenCV REQUIRED)
find_package(OpenImageIO REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glm REQUIRED)


if (APPLE)
    message(STATUS "Looking for MoltenVK package...")
    find_package(MoltenVK CONFIG REQUIRED)
    message(STATUS "MoltenVK_FOUND: ${MoltenVK_FOUND}")
    message(STATUS "MoltenVK_INCLUDE_DIRS: ${MoltenVK_INCLUDE_DIRS}")
    message(STATUS "MoltenVK_LIBRARIES: ${MoltenVK_LIBRARIES}")
endif()


# Remove the option command and use a variable that can be set by Conan
message(STATUS "Initial VALIDATION_LAYERS value: ${VALIDATION_LAYERS}")
# Force the value to False to test
set(VALIDATION_LAYERS OFF)
message(STATUS "After force set VALIDATION_LAYERS value: ${VALIDATION_LAYERS}")

if (APPLE)
    set(LIB_SOURCES
        src/Vertex.h 
        src/VLogger.h 
        src/renderer.h src/renderer.cpp
        src/device.h src/device.cpp 
        src/CommandUtils.h src/CommandUtils.cpp 
        src/RenderEntity.h src/RenderEntity.cpp
        src/OrthographicCamera.h src/OrthographicCamera.cpp
        src/texture.h src/texture.cpp 
        src/TextureTiling.h src/TextureTiling.cpp
        src/MetalSurfaceHelper.h src/MetalSurfaceHelper.mm
        src/MetalHelper.mm
    )
elseif (WIN32)
    set(LIB_SOURCES
        src/Vertex.h 
        src/VLogger.h 
        src/renderer.h src/renderer.cpp
        src/CommandUtils.h src/CommandUtils.cpp 
        src/RenderEntity.h src/RenderEntity.cpp
        src/device.h src/device.cpp 
        src/OrthographicCamera.h src/OrthographicCamera.cpp
        src/texture.h src/texture.cpp 
        src/TextureTiling.h src/TextureTiling.cpp
    )
endif()

# Create the library
if (APPLE)
    add_library(veloxr_lib SHARED ${LIB_SOURCES})
elseif (WIN32)
    add_library(veloxr_lib STATIC ${LIB_SOURCES})
endif()

if (WIN32)
    target_compile_options(veloxr_lib PRIVATE /utf-8)
endif()

add_compile_definitions(VALIDATION_LAYERS_VALUE=$<BOOL:${VALIDATION_LAYERS}>)
message(STATUS "Final VALIDATION_LAYERS value: ${VALIDATION_LAYERS}")
message(STATUS "VALIDATION_LAYERS as bool: $<BOOL:${VALIDATION_LAYERS}>")

set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

if (APPLE)
    target_include_directories(veloxr_lib 
        PUBLIC 
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
            $<BUILD_INTERFACE:${INCLUDE_DIR}>
            $<INSTALL_INTERFACE:include>
        PRIVATE 
            ${Vulkan_INCLUDE_DIRS}
            ${OpenCV_INCLUDE_DIRS}
            ${MoltenVK_INCLUDE_DIRS}
    )
elseif (WIN32)
    target_include_directories(veloxr_lib 
        PUBLIC 
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
            $<BUILD_INTERFACE:${INCLUDE_DIR}>
            $<INSTALL_INTERFACE:include>
        PRIVATE 
            ${Vulkan_INCLUDE_DIRS}
            ${OpenCV_INCLUDE_DIRS}
    )
endif()

if (APPLE)
    target_link_libraries(veloxr_lib
        PUBLIC
            glfw
            Vulkan::Vulkan
            OpenImageIO::OpenImageIO
            opencv::opencv
            glm::glm
            moltenvk::moltenvk
    )
elseif (WIN32)
    target_link_libraries(veloxr_lib
        PUBLIC
            glfw
            Vulkan::Vulkan
            OpenImageIO::OpenImageIO
            opencv::opencv
            glm::glm
    )
endif()


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
            "-framework Metal"
            "-framework MetalKit"
    )
endif()

add_executable(vulkanrenderer src/main.cpp)

if (WIN32)
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

    add_custom_command(TARGET vulkanrenderer POST_BUILD
        COMMAND ${CMAKE_COMMAND} "-DTARGET_FILE=$<TARGET_FILE:vulkanrenderer>" -P "${CMAKE_SOURCE_DIR}/conan/fix_mac_libs.cmake")
endif()
