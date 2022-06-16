cmake_minimum_required(VERSION 3.5)

if(NOT "$ENV{PRODUCT_NAME}" STREQUAL "")
    project($ENV{PRODUCT_NAME} LANGUAGES C CXX)
else()
    project("Graphia" LANGUAGES C CXX)
endif()

find_package(Qt6 COMPONENTS REQUIRED Core)
if(Qt6Core_VERSION VERSION_LESS 6.3.0)
    message(FATAL_ERROR "Qt 6.3 or later required")
endif()

include_directories(source)

# The following sections are only here so that the files are available
# for selection/editing in the IDE (if it supports it); nothing is
# actually built

list(APPEND HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/scripts/defaults.bat
    ${CMAKE_CURRENT_LIST_DIR}/scripts/defaults.sh
    ${CMAKE_CURRENT_LIST_DIR}/scripts/linux-build.sh
    ${CMAKE_CURRENT_LIST_DIR}/scripts/macos-build.sh
    ${CMAKE_CURRENT_LIST_DIR}/scripts/windows-build.bat
)

list(APPEND HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/scripts/clang-tidy.sh
    ${CMAKE_CURRENT_LIST_DIR}/scripts/clazy.sh
    ${CMAKE_CURRENT_LIST_DIR}/scripts/cloc.sh
    ${CMAKE_CURRENT_LIST_DIR}/scripts/compiler-logs-to-table.pl
    ${CMAKE_CURRENT_LIST_DIR}/scripts/cppcheck.cfg
    ${CMAKE_CURRENT_LIST_DIR}/scripts/cppcheck.sh
    ${CMAKE_CURRENT_LIST_DIR}/scripts/parse-compile_commands-json.sh
    ${CMAKE_CURRENT_LIST_DIR}/scripts/qmllint.sh
    ${CMAKE_CURRENT_LIST_DIR}/scripts/upload-symbols.sh
)

list(APPEND HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/installers/linux/build.sh
    ${CMAKE_CURRENT_LIST_DIR}/installers/macos/build.sh
    ${CMAKE_CURRENT_LIST_DIR}/installers/windows/build.bat
    ${CMAKE_CURRENT_LIST_DIR}/installers/windows/installer.nsi
)

list(APPEND HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/.github/workflows/build.yml
)

list(APPEND HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/misc/gdbVisualiser.py
    ${CMAKE_CURRENT_LIST_DIR}/misc/MacOSXBundleInfo.plist.in
)

list(APPEND HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Doxyfile
    ${CMAKE_CURRENT_LIST_DIR}/LICENSE
    ${CMAKE_CURRENT_LIST_DIR}/README.md
    ${CMAKE_CURRENT_LIST_DIR}/.clang-format
    ${CMAKE_CURRENT_LIST_DIR}/.clang-tidy
    ${CMAKE_CURRENT_LIST_DIR}/.gitignore
)

add_subdirectory(source/thirdparty)
add_subdirectory(source/shared)
add_subdirectory(source/app)
add_subdirectory(source/plugins)
add_subdirectory(source/crashreporter)
add_subdirectory(source/messagebox)
add_subdirectory(source/updater)
add_subdirectory(source/updater/editor)

list(REMOVE_DUPLICATES QML_IMPORT_PATH)
set(QML_IMPORT_PATH ${QML_IMPORT_PATH} CACHE STRING "Qt Creator import paths" FORCE)
