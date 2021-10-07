set(CMAKE_CXX_STANDARD 20)

if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-D_DEBUG)
    add_definitions(-DQT_QML_DEBUG)
endif()

set(OpenGL_GL_PREFERENCE "LEGACY")

add_definitions(-DSOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

# https://www.kdab.com/disabling-narrowing-conversions-in-signal-slot-connections/
add_definitions(-DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT)

add_definitions(-DQT_DEPRECATED_WARNINGS)
# disables all the APIs deprecated before Qt 6.0.0
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x060000)

add_definitions(-DAPP_URI="app.graphia")
add_definitions(-DAPP_MINOR_VERSION=0)
add_definitions(-DAPP_MAJOR_VERSION=1)

if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpedantic -Wall -Wextra -Wcast-align -Wcast-qual \
        -Wdisabled-optimization -Wformat=2 -Winit-self \
        -Wmissing-include-dirs -Wold-style-cast -Woverloaded-virtual \
        -Wnon-virtual-dtor -Wredundant-decls -Wshadow")

    # Surprisingly, this actually makes a difference to the pearson correlation code
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -funroll-loops")

    # This tells the linker to export all symbols, even if it thinks they're unused
    # We need it because the plugins require symbols in order for certain things to
    # work e.g. dynamic_cast to QObject from IGraph, under clang
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")

    # Disable ccache if building under travis, as it manages that
    if(NOT DEFINED ENV{TRAVIS})
        # Use ccache, if it's available (https://stackoverflow.com/a/34317588/2721809)
        find_program(CCACHE_EXECUTABLE ccache)
        mark_as_advanced(CCACHE_EXECUTABLE)
        if(CCACHE_EXECUTABLE)
            foreach(LANG C CXX)
                if(NOT DEFINED CMAKE_${LANG}_COMPILER_LAUNCHER AND
                    NOT CMAKE_${LANG}_COMPILER MATCHES ".*/ccache$")
                    message(STATUS "Enabling ccache for ${LANG}")
                    set(CMAKE_${LANG}_COMPILER_LAUNCHER ${CCACHE_EXECUTABLE} CACHE STRING "")
                endif()
            endforeach()
        endif()
    endif()
endif()

if(APPLE)
    # https://stackoverflow.com/questions/52310835
    add_definitions(-D_LIBCPP_DISABLE_AVAILABILITY)
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # GCC
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wlogical-op -Wstrict-null-sentinel \
        -Wdouble-promotion")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    # clang
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wdeprecated")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lc++abi")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -lc++abi")
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

    # Only do VCAnalyze on CI
    if(DEFINED ENV{CI})
        set(VC_ANALYZE_FLAGS "/analyze /analyze:ruleset \
            ${CMAKE_SOURCE_DIR}\\scripts\\VCAnalyze.ruleset")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${VC_ANALYZE_FLAGS}")
    endif()

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} \
        /DYNAMICBASE /NXCOMPAT /MAP /debug")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} \
        /MAP /debug")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
        /permissive- \
        /Zc:rvalueCast /Zc:inline /Zc:strictStrings \
        /Zc:wchar_t /Zc:throwingNew")

    # Assembler
    ENABLE_LANGUAGE(ASM_MASM)
    set(CMAKE_ASM_MASM_FLAGS "/nologo /D_M_X64 /W3 /Cx /Zi")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBUILD_SOURCE_DIR=\\\"${CMAKE_SOURCE_DIR}\\\"")

# Always build with symbols
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g1")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
endif()

find_program(GIT "git")

if (NOT "$ENV{VERSION}" STREQUAL "")
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

    if("${GIT_COMMIT_COUNT}" EQUAL 0 OR "${GIT_BRANCH}" MATCHES "^master|HEAD$")
        set(Version "${GIT_DESCRIBE}")
        add_definitions(-DGIT_DESCRIBE="${GIT_DESCRIBE}")
    else()
        set(Version "${GIT_DESCRIBE}-${GIT_BRANCH}")
        add_definitions(-DGIT_DESCRIBE="${GIT_DESCRIBE}" -DGIT_BRANCH="${GIT_BRANCH}")
    endif()
else()
    set(Version "unknown")
endif()

add_definitions(-DVERSION="${Version}")

if (NOT "$ENV{PUBLISHER}" STREQUAL "")
    set(Publisher $ENV{PUBLISHER})
else()
    set(Publisher "Graphia Technologies Ltd.")
endif()

string(TIMESTAMP CURRENT_YEAR "%Y")
set(Copyright "\(c\) 2013-${CURRENT_YEAR} ${Publisher}")
add_definitions(-DCOPYRIGHT="${Copyright}")

string(TOLOWER "${PROJECT_NAME}" NativeExtension)

include_directories(${CMAKE_CURRENT_LIST_DIR})
