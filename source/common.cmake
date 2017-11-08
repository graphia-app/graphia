set(CMAKE_CXX_STANDARD 14)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-D_DEBUG)
endif()

# https://www.kdab.com/disabling-narrowing-conversions-in-signal-slot-connections/
add_definitions(-DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT)

add_definitions(-DQT_DEPRECATED_WARNINGS)
# disables all the APIs deprecated before Qt 6.0.0
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x060000)

add_definitions(-DAPP_URI="com.kajeka")
add_definitions(-DAPP_MINOR_VERSION=0)
add_definitions(-DAPP_MAJOR_VERSION=1)

if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpedantic -Wall -Wextra -Wcast-align -Wcast-qual \
        -Wdisabled-optimization -Wformat=2 -Winit-self \
        -Wmissing-include-dirs -Wold-style-cast -Woverloaded-virtual \
        -Wnon-virtual-dtor -Wredundant-decls -Wshadow")

    # Surprisingly, this actually makes a difference to the pearson correlation code
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -funroll-loops")
endif()

# GCC specific warnings
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wlogical-op -Wstrict-null-sentinel \
        -Wdouble-promotion")
endif()

if(MSVC)
    add_definitions(-DUNICODE -D_UNICODE)

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4250 /wd4996")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} \
        /DYNAMICBASE /NXCOMPAT /MAP /debug /opt:ref")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
        /Zc:rvalueCast /Zc:inline /Zc:strictStrings \
        /Zc:wchar_t /Zc:throwingNew")

    # Assembler
    ENABLE_LANGUAGE(ASM_MASM)
    set(CMAKE_ASM_MASM_FLAGS "/nologo /D_M_X64 /W3 /Cx /Zi")
endif()

# Always build with symbols
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
endif()

if (NOT "$ENV{VERSION}" STREQUAL "")
    set(Version $ENV{VERSION})
else()
    set(Version "development")
endif()

add_definitions(-DVERSION="${Version}")

if (NOT "$ENV{COPYRIGHT}" STREQUAL "")
    set(Copyright $ENV{COPYRIGHT})
else()
    set(Copyright "\(c\) Copyright notice")
endif()

add_definitions(-DCOPYRIGHT="${Copyright}")

include_directories(${CMAKE_CURRENT_LIST_DIR})
