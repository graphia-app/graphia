cmake_minimum_required(VERSION 3.19)

if(NOT "$ENV{PRODUCT_NAME}" STREQUAL "")
    project($ENV{PRODUCT_NAME} LANGUAGES C CXX ASM)
else()
    project("Graphia" LANGUAGES C CXX ASM)
endif()

find_package(Qt6 REQUIRED COMPONENTS Core)
qt_standard_project_setup(REQUIRES 6.9)

include_directories(source)

# Sanitizers to build everything with, e.g. "address;undefined"
set(GRAPHIA_SANITIZERS "" CACHE STRING "Sanitizers to build with, e.g. address;undefined")
if(GRAPHIA_SANITIZERS AND NOT MSVC)
    list(JOIN GRAPHIA_SANITIZERS "," SANITIZERS)
    add_compile_options(-fsanitize=${SANITIZERS} -fno-sanitize-recover=all -fno-omit-frame-pointer)
    add_link_options(-fsanitize=${SANITIZERS})
endif()

add_subdirectory(source/thirdparty)

# Clang's source based code coverage, for everything except third party code;
# run the tests with LLVM_PROFILE_FILE set, then use llvm-profdata and llvm-cov
option(GRAPHIA_COVERAGE "Build with Clang source based code coverage instrumentation" OFF)
if(GRAPHIA_COVERAGE)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        message(FATAL_ERROR "GRAPHIA_COVERAGE requires Clang")
    endif()

    add_compile_options(-fprofile-instr-generate -fcoverage-mapping)
    add_link_options(-fprofile-instr-generate)
endif()

add_subdirectory(source/shared)
add_subdirectory(source/plugins)
add_subdirectory(source/app)

if(NOT EMSCRIPTEN)
    add_subdirectory(source/crashreporter)
    add_subdirectory(source/messagebox)
    add_subdirectory(source/updater)
    add_subdirectory(source/updater/editor)
endif()

if(NOT EMSCRIPTEN)
    option(GRAPHIA_BUILD_TESTS "Build the unit tests" ON)
else()
    set(GRAPHIA_BUILD_TESTS OFF)
endif()

if(GRAPHIA_BUILD_TESTS)
    enable_testing()
    add_subdirectory(source/tests)
endif()

# The following sections are only here so that the files are available
# for selection/editing in the IDE (if it supports it); nothing is
# actually built

list(APPEND MISC
    ${CMAKE_CURRENT_LIST_DIR}/scripts/defaults.ps1
    ${CMAKE_CURRENT_LIST_DIR}/scripts/defaults.sh
    ${CMAKE_CURRENT_LIST_DIR}/scripts/linux-build.sh
    ${CMAKE_CURRENT_LIST_DIR}/scripts/macos-build.sh
    ${CMAKE_CURRENT_LIST_DIR}/scripts/wasm-build.sh
    ${CMAKE_CURRENT_LIST_DIR}/scripts/wasm-deploy.sh
    ${CMAKE_CURRENT_LIST_DIR}/scripts/windows-build.ps1
)

list(APPEND MISC
    ${CMAKE_CURRENT_LIST_DIR}/scripts/clang-tidy.sh
    ${CMAKE_CURRENT_LIST_DIR}/scripts/clazy.sh
    ${CMAKE_CURRENT_LIST_DIR}/scripts/cloc.sh
    ${CMAKE_CURRENT_LIST_DIR}/scripts/cppcheck.cfg
    ${CMAKE_CURRENT_LIST_DIR}/scripts/cppcheck.sh
    ${CMAKE_CURRENT_LIST_DIR}/scripts/emsdk-version-for-qt-version.sh
    ${CMAKE_CURRENT_LIST_DIR}/scripts/iwyu.sh
    ${CMAKE_CURRENT_LIST_DIR}/scripts/parse-compile_commands-json.sh
    ${CMAKE_CURRENT_LIST_DIR}/scripts/qmllint.sh
    ${CMAKE_CURRENT_LIST_DIR}/scripts/static-analysis.sh
    ${CMAKE_CURRENT_LIST_DIR}/scripts/tidy-headers.sh
    ${CMAKE_CURRENT_LIST_DIR}/scripts/upload-symbols.sh
)

list(APPEND MISC
    ${CMAKE_CURRENT_LIST_DIR}/installers/linux/build.sh
    ${CMAKE_CURRENT_LIST_DIR}/installers/macos/build.sh
    ${CMAKE_CURRENT_LIST_DIR}/installers/wasm/build.sh
    ${CMAKE_CURRENT_LIST_DIR}/installers/windows/installer.nsi
    ${CMAKE_CURRENT_LIST_DIR}/installers/windows/make-installer.ps1
    ${CMAKE_CURRENT_LIST_DIR}/installers/windows/prepare-installer.ps1
)

list(APPEND MISC
    ${CMAKE_CURRENT_LIST_DIR}/.github/workflows/build.yml
)

list(APPEND MISC
    ${CMAKE_CURRENT_LIST_DIR}/misc/gdbVisualiser.py
    ${CMAKE_CURRENT_LIST_DIR}/misc/MacOSXBundleInfo.plist.in
)

list(APPEND MISC
    ${CMAKE_CURRENT_LIST_DIR}/Doxyfile
    ${CMAKE_CURRENT_LIST_DIR}/LICENSE
    ${CMAKE_CURRENT_LIST_DIR}/README.md
    ${CMAKE_CURRENT_LIST_DIR}/.clang-format
    ${CMAKE_CURRENT_LIST_DIR}/.clang-tidy
    ${CMAKE_CURRENT_LIST_DIR}/.gitignore
)

add_custom_target(Misc SOURCES ${MISC})

list(REMOVE_DUPLICATES QML_IMPORT_PATH)
set(QML_IMPORT_PATH ${QML_IMPORT_PATH} CACHE STRING "Qt Creator import paths" FORCE)
