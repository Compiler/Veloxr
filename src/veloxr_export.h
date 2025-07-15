#pragma once

// Windows DLL export/import macros
#ifdef _WIN32
    #ifdef BUILD_SHARED_LIBS
        #ifdef VELOXR_LIBRARY_BUILD
            #define VELOXR_API __declspec(dllexport)
        #else
            #define VELOXR_API __declspec(dllimport)
        #endif
    #else
        #define VELOXR_API
    #endif
#else
    // Non-Windows platforms
    #define VELOXR_API
#endif 