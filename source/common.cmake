set(CMAKE_CXX_STANDARD 14)

add_definitions(-DAPP_URI="com.kajeka")
add_definitions(-DAPP_MINOR_VERSION=0)
add_definitions(-DAPP_MAJOR_VERSION=1)

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
