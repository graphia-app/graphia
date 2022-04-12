include_directories(${CMAKE_CURRENT_LIST_DIR})

list(APPEND SHARED_THIRDPARTY_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/qqmlsortfilterproxymodel.h
    ${CMAKE_CURRENT_LIST_DIR}/qvariantlessthan.h

    ${CMAKE_CURRENT_LIST_DIR}/filters/alloffilter.h
    ${CMAKE_CURRENT_LIST_DIR}/filters/anyoffilter.h
    ${CMAKE_CURRENT_LIST_DIR}/filters/expressionfilter.h
    ${CMAKE_CURRENT_LIST_DIR}/filters/filtercontainerfilter.h
    ${CMAKE_CURRENT_LIST_DIR}/filters/filtercontainer.h
    ${CMAKE_CURRENT_LIST_DIR}/filters/filter.h
    ${CMAKE_CURRENT_LIST_DIR}/filters/indexfilter.h
    ${CMAKE_CURRENT_LIST_DIR}/filters/rangefilter.h
    ${CMAKE_CURRENT_LIST_DIR}/filters/regexpfilter.h
    ${CMAKE_CURRENT_LIST_DIR}/filters/rolefilter.h
    ${CMAKE_CURRENT_LIST_DIR}/filters/valuefilter.h

    ${CMAKE_CURRENT_LIST_DIR}/proxyroles/expressionrole.h
    ${CMAKE_CURRENT_LIST_DIR}/proxyroles/filterrole.h
    ${CMAKE_CURRENT_LIST_DIR}/proxyroles/joinrole.h
    ${CMAKE_CURRENT_LIST_DIR}/proxyroles/proxyrolecontainer.h
    ${CMAKE_CURRENT_LIST_DIR}/proxyroles/proxyrole.h
    ${CMAKE_CURRENT_LIST_DIR}/proxyroles/regexprole.h
    ${CMAKE_CURRENT_LIST_DIR}/proxyroles/singlerole.h
    ${CMAKE_CURRENT_LIST_DIR}/proxyroles/switchrole.h

    ${CMAKE_CURRENT_LIST_DIR}/sorters/expressionsorter.h
    ${CMAKE_CURRENT_LIST_DIR}/sorters/filtersorter.h
    ${CMAKE_CURRENT_LIST_DIR}/sorters/rolesorter.h
    ${CMAKE_CURRENT_LIST_DIR}/sorters/sortercontainer.h
    ${CMAKE_CURRENT_LIST_DIR}/sorters/sorter.h
    ${CMAKE_CURRENT_LIST_DIR}/sorters/stringsorter.h
)

list(APPEND SHARED_THIRDPARTY_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/qqmlsortfilterproxymodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/qvariantlessthan.cpp

    ${CMAKE_CURRENT_LIST_DIR}/filters/alloffilter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/filters/anyoffilter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/filters/expressionfilter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/filters/filtercontainerfilter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/filters/filtercontainer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/filters/filter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/filters/filtersqmltypes.cpp
    ${CMAKE_CURRENT_LIST_DIR}/filters/indexfilter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/filters/rangefilter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/filters/regexpfilter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/filters/rolefilter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/filters/valuefilter.cpp

    ${CMAKE_CURRENT_LIST_DIR}/proxyroles/expressionrole.cpp
    ${CMAKE_CURRENT_LIST_DIR}/proxyroles/filterrole.cpp
    ${CMAKE_CURRENT_LIST_DIR}/proxyroles/joinrole.cpp
    ${CMAKE_CURRENT_LIST_DIR}/proxyroles/proxyrolecontainer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/proxyroles/proxyrole.cpp
    ${CMAKE_CURRENT_LIST_DIR}/proxyroles/proxyrolesqmltypes.cpp
    ${CMAKE_CURRENT_LIST_DIR}/proxyroles/regexprole.cpp
    ${CMAKE_CURRENT_LIST_DIR}/proxyroles/singlerole.cpp
    ${CMAKE_CURRENT_LIST_DIR}/proxyroles/switchrole.cpp

    ${CMAKE_CURRENT_LIST_DIR}/sorters/expressionsorter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/sorters/filtersorter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/sorters/rolesorter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/sorters/sortercontainer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/sorters/sorter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/sorters/sortersqmltypes.cpp
    ${CMAKE_CURRENT_LIST_DIR}/sorters/stringsorter.cpp
)
