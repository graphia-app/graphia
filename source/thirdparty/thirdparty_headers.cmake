include_directories(${CMAKE_CURRENT_LIST_DIR})
include_directories(${CMAKE_CURRENT_LIST_DIR}/blaze)
include_directories(${CMAKE_CURRENT_LIST_DIR}/boost/spirit)
include_directories(${CMAKE_CURRENT_LIST_DIR}/json)
include_directories(${CMAKE_CURRENT_LIST_DIR}/matio)
include_directories(${CMAKE_CURRENT_LIST_DIR}/zlib)

add_definitions(-DQCUSTOMPLOT_USE_OPENGL -DQCUSTOMPLOT_USE_LIBRARY)
add_definitions(-DCRYPTOPP_DISABLE_ASM)
