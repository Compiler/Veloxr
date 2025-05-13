# Get otool output
execute_process(COMMAND arch -arm64 otool -L ${TARGET_FILE}
    OUTPUT_VARIABLE OTOOL_OUTPUT)

# Gate debug logs to Azure build
if(DEFINED ENV{BUILD_REASON})
    message(STATUS "Raw otool output: ${OTOOL_OUTPUT}")
endif()

# Convert output to a list of strings, one per line
# Note that libraries will have a tab character in front of them
string(REPLACE "\n" ";" OTOOL_LIST ${OTOOL_OUTPUT})

# Create a list for library paths that need fixing
set(TO_FIX_LIST "")
# Figure out which lines we need to keep
foreach(OTOOL_LIB IN LISTS OTOOL_LIST)
    if(${OTOOL_LIB} MATCHES "^\t")
        string(STRIP "${OTOOL_LIB}" OTOOL_LIB)

        if(DEFINED ENV{BUILD_REASON})
            message(STATUS "Checking otool library: '${OTOOL_LIB}'")
        endif()
        
        if(${OTOOL_LIB} MATCHES ".*\\/opt.*|.*\\/Cellar.*|.*\\/Users.*|^lib|^@executable_path|^[^/]"
                AND NOT ${OTOOL_LIB} MATCHES "libclang_rt")
            if(DEFINED ENV{BUILD_REASON})
                message(STATUS "Library should be fixed: '${OTOOL_LIB}'")
            endif()
            string(REPLACE " " ";" OTOOL_LIB_SPLIT ${OTOOL_LIB})
            list(GET OTOOL_LIB_SPLIT 0 OTOOL_LIB_NAME)
            # Append just the library path/name to our fix list
            list(APPEND TO_FIX_LIST ${OTOOL_LIB_NAME})
        elseif(DEFINED ENV{BUILD_REASON})
            # Gate debug logs to Azure build
            message(STATUS "Dropping non-matching otool library: ${OTOOL_LIB}")
        endif()
    else()
        message(STATUS "Library does not start with tab: '${OTOOL_LIB}'")
    endif()
endforeach()

message(STATUS "Otool output: ${TO_FIX_LIST}")

get_filename_component(TARGET_BASENAME ${TARGET_FILE} NAME)

foreach(BAD_LIB IN LISTS TO_FIX_LIST)
    get_filename_component(BASENAME ${BAD_LIB} NAME)
    message("Handling basename: ${BASENAME}")

    if(TARGET_BASENAME STREQUAL "${BASENAME}")
        # Set identification name to the library basename
        execute_process(COMMAND 
            arch -arm64 install_name_tool -id "@executable_path/${BASENAME}" ${TARGET_FILE})
    elseif(${BAD_LIB} MATCHES ".*\\.framework")
        # Handle Qt Frameworks
        # Get the directory portion of the frameworks path
        # Example: @rpath/QtCore.framework/Versions/A/QtCore -> @rpath
        string(REPLACE ".framework" ";" FRAMEWORK_SPLIT ${BAD_LIB})
        list(GET FRAMEWORK_SPLIT 0 FRAMEWORK_DIR_PART)
        get_filename_component(FRAMEWORK_DIR ${FRAMEWORK_DIR_PART} DIRECTORY)

        # Replace parent path with fixed path to executable directory
        string(REPLACE ${FRAMEWORK_DIR} "@executable_path" FIXED_LIB ${BAD_LIB})

        message(STATUS "Fixing Qt Framework: '${BAD_LIB}' => '${FIXED_LIB}'")
        # Fix the rpath by changing it from the old to the new fixed version
        # Example: @rpath/QtCore.framework/Versions/A/QtCore -> @executable_path/QtCore.framework/Versions/A/QtCore
        execute_process(COMMAND 
            arch -arm64 install_name_tool -change ${BAD_LIB} ${FIXED_LIB} ${TARGET_FILE})
    else()
        # Handle everything else (bare libs, libs in /Users, etc)
        # Get the base directory for the bad lib
        get_filename_component(LIB_DIR ${BAD_LIB} DIRECTORY)

        if (LIB_DIR STREQUAL "")
            # If the directory is empty, then just prepend the executable path stuff
            set(FIXED_LIB "@executable_path/${BAD_LIB}")
        else()
            # If the directory is not empty, then we need to replace the bad parent directory with the executable stuff
            string(REPLACE ${LIB_DIR} "@executable_path" FIXED_LIB ${BAD_LIB})
        endif()
        
        # Fix the rpath
        # Example: libOpenImageIO.2.3.dylib -> @executable_path/libOpenImageIO.2.3.dylib
        message(STATUS "Doing install_name_tool -change '${BAD_LIB}' '${FIXED_LIB}' '${TARGET_FILE}'")
        execute_process(COMMAND 
            arch -arm64 install_name_tool -change ${BAD_LIB} ${FIXED_LIB} ${TARGET_FILE})
    endif()
endforeach()

# Separate debug information
execute_process(COMMAND arch -arm64 dsymutil ${TARGET_FILE})
execute_process(COMMAND strip -S ${TARGET_FILE})

# Gate debug logs to Azure build
if(DEFINED ENV{BUILD_REASON})
    # Get otool output (again)
    execute_process(COMMAND arch -arm64 otool -L ${TARGET_FILE}
        OUTPUT_VARIABLE OTOOL_OUTPUT)

    message(STATUS "Raw otool output after fixing: ${OTOOL_OUTPUT}")
endif()
