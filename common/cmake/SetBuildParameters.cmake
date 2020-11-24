set(UNIVERSAL_BUILD FALSE CACHE BOOL "Indigo C++11 universal build without dependency on libstdc++")

# Use ccache if possible
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    message(STATUS "Using ccache: ${CCACHE_PROGRAM}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

if(NOT MSVC AND NOT BINGO)
    set(VISIBILITY_HIDDEN YES)
endif()

if (NOT CMAKE_BUILD_TYPE)
    if(NOT BUILD_DEBUG)
        message(STATUS "Set CMAKE_BUILD_TYPE to Release")
        set(CMAKE_BUILD_TYPE Release)
    else()
        message(STATUS "Set CMAKE_BUILD_TYPE to Debug")
        set(CMAKE_BUILD_TYPE Debug)
    endif()
endif()

if(MSVC)
    add_definitions(/D "_CRT_SECURE_NO_WARNINGS")
    if (CMAKE_BUILD_TYPE MATCHES Debug)
        add_definitions(/Z7)
    endif()
elseif(UNIX AND NOT APPLE OR MINGW)
    if (CMAKE_SYSTEM_PROCESSOR MATCHES "^(amd64|i.86|powerpc|ppc|sparc|x86_64)")
        if (SUBSYSTEM_NAME MATCHES "x86")
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32 -D_FILE_OFFSET_BITS=64")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32 -D_FILE_OFFSET_BITS=64")
        elseif(SUBSYSTEM_NAME MATCHES "x64")
            if(CMAKE_SYSTEM_NAME MATCHES "Linux")
                set(CMAKE_C_FLAGS "-include ${CMAKE_CURRENT_LIST_DIR}/../hacks/gcc_preinclude.h ${CMAKE_C_FLAGS}")
                set(CMAKE_CXX_FLAGS "-include ${CMAKE_CURRENT_LIST_DIR}/../hacks/gcc_preinclude.h ${CMAKE_CXX_FLAGS}")
            endif()
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
        endif()
    endif()

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

    if (DEFINED ENV{UNIVERSAL} OR UNIVERSAL_BUILD)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++")
    endif()
elseif(APPLE)
    include(MacFrameworks)

    set(CMAKE_OSX_ARCHITECTURES "x86_64")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -arch x86_64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch x86_64 -std=c++11 -stdlib=libc++")

    set(CMAKE_OSX_DEPLOYMENT_TARGET ${SUBSYSTEM_NAME})
    set(SDK_SUBSYSTEM_NAME ${SUBSYSTEM_NAME})
    message(STATUS "Deployment target: ${CMAKE_OSX_DEPLOYMENT_TARGET}")

    if (DEFINED ENV{UNIVERSAL} OR UNIVERSAL_BUILD)
        set(CMAKE_OSX_SYSROOT /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk)
    else()
        set(CMAKE_OSX_SYSROOT /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX${SDK_SUBSYSTEM_NAME}.sdk)
    endif()
    
    message(STATUS "SDK: ${CMAKE_OSX_SYSROOT}")
endif()

if(UNIX OR APPLE)
    set(COMPILE_FLAGS "${COMPILE_FLAGS} -fPIC")

    #Set RPATH
    set(CMAKE_SKIP_BUILD_RPATH  FALSE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
    set(CMAKE_MACOSX_RPATH ON)
    if(APPLE)
            set(CMAKE_INSTALL_RPATH "\@loader_path")
    else()
            set(CMAKE_INSTALL_RPATH "\$ORIGIN")
    endif()
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()

if(VISIBILITY_HIDDEN)
    set(COMPILE_FLAGS "${COMPILE_FLAGS} -fvisibility=hidden")
endif()

if(MINGW)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -std=c11")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -std=c++11")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lc++abi")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lc++abi")
        message(STATUS "Clang: using libc++")
    endif()
endif()

message(STATUS "CMAKE_C_FLAGS ${CMAKE_C_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS}")

#find_program(ClangTidy_Command clang-tidy)
#if (ClangTidy_Command)
#    message(STATUS "Found clang-tidy at ${ClangTidy_Command}, using it for static analysis")
#    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
#    set(CMAKE_CXX_CLANG_TIDY ${ClangTidy_Command})
#    include_directories(SYSTEM $ENV{LIBCPP_INCLUDE_PATH})
#endif()
