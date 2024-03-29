include_directories(SYSTEM ${CMAKE_CURRENT_LIST_DIR})
include_directories(SYSTEM ${CMAKE_CURRENT_LIST_DIR}/blaze)
include_directories(SYSTEM ${CMAKE_CURRENT_LIST_DIR}/boost/spirit)
include_directories(SYSTEM ${CMAKE_CURRENT_LIST_DIR}/json)
include_directories(SYSTEM ${CMAKE_CURRENT_LIST_DIR}/hdf5)
include_directories(SYSTEM ${CMAKE_CURRENT_LIST_DIR}/matio)
include_directories(SYSTEM ${CMAKE_CURRENT_LIST_DIR}/zlib)
include_directories(SYSTEM ${CMAKE_CURRENT_LIST_DIR}/qcustomplot)
include_directories(SYSTEM ${CMAKE_CURRENT_LIST_DIR}/breakpad/src)
include_directories(SYSTEM ${CMAKE_CURRENT_LIST_DIR}/utfcpp/source)

add_definitions(-DQCUSTOMPLOT_USE_OPENGL -DQCUSTOMPLOT_USE_LIBRARY)
add_definitions(-DCRYPTOPP_DISABLE_ASM)
add_definitions(-DBPLOG_MINIMUM_SEVERITY=SEVERITY_CRITICAL)
add_definitions(-D__STDC_FORMAT_MACROS) # For breakpad_types.h
