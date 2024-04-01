set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-D_DEBUG)
    add_definitions(-DQT_QML_DEBUG)
endif()

set(OpenGL_GL_PREFERENCE "LEGACY")

add_definitions(-DSOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

# https://www.kdab.com/disabling-narrowing-conversions-in-signal-slot-connections/
add_definitions(-DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT)

# Disable all deprecated APIs at or before the Qt version being used
string(REGEX MATCHALL "[0-9]+" QT_VERSION_LIST "${Qt6Core_VERSION}")
list(GET QT_VERSION_LIST 0 QT_MAJOR)
list(GET QT_VERSION_LIST 1 QT_MINOR)
list(GET QT_VERSION_LIST 2 QT_PATCH)
math(EXPR QT_VERSION_HEX "(${QT_MAJOR} << 16) + (${QT_MINOR} << 8) + ${QT_PATCH}")
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=${QT_VERSION_HEX})

add_definitions(-DAPP_URI="app.graphia")
add_definitions(-DAPP_MINOR_VERSION=0)
add_definitions(-DAPP_MAJOR_VERSION=1)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/build_defines.h "// This file contains defines created by the build system\n \
    \n// NOLINTBEGIN(cppcoreguidelines-macro-usage)\n\n")

if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpedantic -Wall -Wextra -Wcast-align -Wcast-qual \
        -Wdisabled-optimization -Wformat=2 -Winit-self \
        -Wmissing-include-dirs -Wold-style-cast -Woverloaded-virtual \
        -Wnon-virtual-dtor -Wredundant-decls -Wshadow -Wconversion")

    # Surprisingly, this actually makes a difference to the pearson correlation code
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -funroll-loops")

    # This tells the linker to export all symbols, even if it thinks they're unused
    # We need it because the plugins require symbols in order for certain things to
    # work e.g. dynamic_cast to QObject from IGraph, under clang
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")

    find_program(MOLD_EXECUTABLE mold)
    mark_as_advanced(MOLD_EXECUTABLE)
    if(MOLD_EXECUTABLE)
        message(STATUS "Enabling mold: ${MOLD_EXECUTABLE}")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=mold")
    endif()
endif()

# Use ccache, if it's available (https://stackoverflow.com/a/34317588/2721809)
find_program(CCACHE_EXECUTABLE NAMES sccache ccache)
mark_as_advanced(CCACHE_EXECUTABLE)
if(CCACHE_EXECUTABLE)
    foreach(LANG C CXX)
        if(NOT DEFINED CMAKE_${LANG}_COMPILER_LAUNCHER AND
            NOT CMAKE_${LANG}_COMPILER MATCHES ".*/ccache$")
            message(STATUS "Enabling ${CCACHE_EXECUTABLE} for ${LANG}")
            set(CMAKE_${LANG}_COMPILER_LAUNCHER ${CCACHE_EXECUTABLE} CACHE STRING "")
        endif()
    endforeach()
endif()

if(APPLE)
    # https://stackoverflow.com/questions/52310835
    add_definitions(-D_LIBCPP_DISABLE_AVAILABILITY)
endif()

# C++17 notionally removed std::unary_function
# Boost 1.67 depends on std::unary_function
# Contemporary Boost removes that dependency but needs changes to boost_spirit_qstring_adapter.h
# Recent versions of libcpp *actually* remove std::unary_function ...unless this define is added
add_definitions(-D_LIBCPP_ENABLE_CXX17_REMOVED_UNARY_BINARY_FUNCTION)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # GCC
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wlogical-op -Wstrict-null-sentinel \
        -Wdouble-promotion")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    # clang
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wdeprecated")
endif()

if(MSVC)
    add_definitions(-DUNICODE -D_UNICODE)

    # Enable some warnings
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")

    # MSVC is picky with UTF8 files recognition so force it on
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")

    # Disable large numbers of encoding warnings for boost with utf8 encoding on
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4828")

    # Disable inherits via dominance
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4250")

    # Disable deprecated warnings
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4996")

    if(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        # Only do MSVC code analysis on CI
        if(DEFINED ENV{CI})
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /analyze")
        endif()

        # Clang-cl doesn't seem to understand this
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:throwingNew")
    endif()

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} \
        /DYNAMICBASE /NXCOMPAT /MAP /debug")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} \
        /MAP /debug")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
        /permissive- \
        /Zc:rvalueCast /Zc:inline /Zc:strictStrings \
        /Zc:wchar_t")

    # Assembler
    ENABLE_LANGUAGE(ASM_MASM)
    set(CMAKE_ASM_MASM_FLAGS "/nologo /D_M_X64 /W3 /Cx /Z7")
endif()

if(EMSCRIPTEN)
    set(LINK_TYPE STATIC)
    set(QT_WASM_PTHREAD_POOL_SIZE navigator.hardwareConcurrency)
    add_definitions(-DDISABLE_EXCEPTION_THROWING)
    add_definitions(-DOPENGL_ES)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBUILD_SOURCE_DIR=\\\"${CMAKE_SOURCE_DIR}\\\"")

# Always build with symbols
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Z7")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g1")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
endif()

if(NOT DEFINED LINK_TYPE)
    set(LINK_TYPE SHARED)
endif()

if(LINK_TYPE STREQUAL STATIC)
    add_definitions(-DSTATIC_LINKING)
endif()

find_program(GIT "git")

if(NOT "$ENV{VERSION}" STREQUAL "")
    set(Version $ENV{VERSION})
elseif(GIT)
    execute_process(COMMAND ${GIT} -C ${CMAKE_SOURCE_DIR} describe --always
        OUTPUT_VARIABLE GIT_DESCRIBE OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND ${GIT} -C ${CMAKE_SOURCE_DIR} describe --abbrev=0
        OUTPUT_VARIABLE GIT_TAG OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND ${GIT} -C ${CMAKE_SOURCE_DIR} rev-list ${GIT_TAG}..HEAD --count
        OUTPUT_VARIABLE GIT_COMMIT_COUNT OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND ${GIT} -C ${CMAKE_SOURCE_DIR} rev-parse --abbrev-ref HEAD
        OUTPUT_VARIABLE GIT_BRANCH OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND ${GIT} -C ${CMAKE_SOURCE_DIR} rev-parse --short HEAD
        OUTPUT_VARIABLE GIT_SHA OUTPUT_STRIP_TRAILING_WHITESPACE)

    file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/build_defines.h "#define GIT_DESCRIBE \"${GIT_DESCRIBE}\"\n")
    file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/build_defines.h "#define GIT_SHA \"${GIT_SHA}\"\n")

    if("${GIT_COMMIT_COUNT}" EQUAL 0 OR "${GIT_BRANCH}" MATCHES "^master|HEAD$")
        set(Version "${GIT_DESCRIBE}")
    else()
        set(Version "${GIT_DESCRIBE}-${GIT_BRANCH}")
        file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/build_defines.h "#define GIT_BRANCH \"${GIT_BRANCH}\"\n")
    endif()
else()
    set(Version "unknown")
endif()

file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/build_defines.h "#define VERSION \"${Version}\"\n")

if (NOT "$ENV{PUBLISHER}" STREQUAL "")
    set(Publisher $ENV{PUBLISHER})
else()
    set(Publisher "Graphia Technologies Ltd.")
endif()

string(TIMESTAMP CURRENT_YEAR "%Y")
set(Copyright "\(c\) 2013-${CURRENT_YEAR} ${Publisher}")
file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/build_defines.h "#define COPYRIGHT \"${Copyright}\"\n \
    \n// NOLINTEND(cppcoreguidelines-macro-usage)\n")

string(TOLOWER "${PROJECT_NAME}" NativeExtension)

include_directories(${CMAKE_CURRENT_LIST_DIR})
