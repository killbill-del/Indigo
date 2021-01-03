if(MSVC OR MINGW)
    set(SYSTEM_NAME "Win")
    if (NOT SUBSYSTEM_NAME)
        if(CMAKE_SIZEOF_VOID_P MATCHES 8)
            set(SUBSYSTEM_NAME "x64")
        else()
            set(SUBSYSTEM_NAME "x86")
        ENDIF()
    endif()
    set(SYSTEM_DL_EXTENSION ".dll")
elseif(APPLE)
    set(SYSTEM_NAME "Mac")
    if (NOT SUBSYSTEM_NAME)
        EXEC_PROGRAM(uname ARGS -v  OUTPUT_VARIABLE DARWIN_VERSION)
        string(REGEX MATCH "[0-9]+" DARWIN_VERSION ${DARWIN_VERSION})
        if(DARWIN_VERSION MATCHES 11)
            set(SUBSYSTEM_NAME "10.7")
        elseif(DARWIN_VERSION MATCHES 12)
            set(SUBSYSTEM_NAME "10.8")
        elseif(DARWIN_VERSION MATCHES 13)
            set(SUBSYSTEM_NAME "10.9")
        elseif(DARWIN_VERSION MATCHES 14)
            set(SUBSYSTEM_NAME "10.10")
        elseif(DARWIN_VERSION MATCHES 15)
            set(SUBSYSTEM_NAME "10.11")
        elseif(DARWIN_VERSION MATCHES 16)
            set(SUBSYSTEM_NAME "10.12")
        elseif(DARWIN_VERSION MATCHES 17)
            set(SUBSYSTEM_NAME "10.13")
        elseif(DARWIN_VERSION MATCHES 18)
            set(SUBSYSTEM_NAME "10.14")
        elseif(DARWIN_VERSION MATCHES 19)
            set(SUBSYSTEM_NAME "10.15")
        elseif(DARWIN_VERSION MATCHES 20)
            set(SUBSYSTEM_NAME "11.0")
        else()
            message(FATAL_ERROR "Unsupported DARWIN_VERSION: ${DARWIN_VERSION}")
        endif()
    endif()
    set(SYSTEM_DL_EXTENSION ".dylib")
elseif(UNIX)
    set(SYSTEM_NAME "Linux")
    if (NOT SUBSYSTEM_NAME)
        if(CMAKE_SIZEOF_VOID_P MATCHES 8)
            set(SUBSYSTEM_NAME "x64")
        else()
            set(SUBSYSTEM_NAME "x86")
        endif()
    endif()
    set(SYSTEM_DL_EXTENSION ".so")
else()
   MESSAGE(FATAL_ERROR "Unsupported system")
endif()


if (SYSTEM_NAME MATCHES "Mac")
    set(PACKAGE_SUFFIX "mac${SUBSYSTEM_NAME}")
else()
    if (SYSTEM_NAME MATCHES "Win")
        set(PACKAGE_SUFFIX_PREFIX "win")
    elseif (SYSTEM_NAME MATCHES "Linux")
        set(PACKAGE_SUFFIX_PREFIX "linux")
    else()
        MESSAGE(FATAL_ERROR "Unsupported system")
    endif()
    if (SUBSYSTEM_NAME MATCHES "x86")
        set(PACKAGE_SUFFIX_SUFFIX "32")
    elseif (SUBSYSTEM_NAME MATCHES "x64")
        set(PACKAGE_SUFFIX_SUFFIX "64")
    else()
        MESSAGE(FATAL_ERROR "Unsupported system")
    endif()
    set(PACKAGE_SUFFIX "${PACKAGE_SUFFIX_PREFIX}${PACKAGE_SUFFIX_SUFFIX}")
endif()

message(STATUS "System-specific folder name: ${SYSTEM_NAME}")
message(STATUS "Subsystem-specific folder name: ${SUBSYSTEM_NAME}")

macro(LIBRARY_NAME LIBRARY_BASENAME)
    set(LIBRARY_NAME_RESULT "")
    if(NOT MSVC AND NOT MINGW)
        set(LIBRARY_NAME_RESULT "lib")
    endif()
    set(LIBRARY_NAME_RESULT ${LIBRARY_NAME_RESULT}${LIBRARY_BASENAME}${SYSTEM_DL_EXTENSION})
    set(${LIBRARY_BASENAME}_NAME ${LIBRARY_NAME_RESULT})
endmacro()

