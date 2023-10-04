add_definitions(-DQCUSTOMPLOT_COMPILE_LIBRARY)

list(APPEND SHARED_THIRDPARTY_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/qcustomplot.h
    ${CMAKE_CURRENT_LIST_DIR}/qcustomplotcolorprovider.h
    ${CMAKE_CURRENT_LIST_DIR}/qcustomplotquickitem.h
)

list(APPEND SHARED_THIRDPARTY_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/qcustomplot.cpp
    ${CMAKE_CURRENT_LIST_DIR}/qcustomplotcolorprovider.cpp
    ${CMAKE_CURRENT_LIST_DIR}/qcustomplotquickitem.cpp
)
