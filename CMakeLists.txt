cmake_minimum_required(VERSION 3.19)

if(NOT "$ENV{PRODUCT_NAME}" STREQUAL "")
    project($ENV{PRODUCT_NAME} LANGUAGES C CXX ASM)
else()
    project("Graphia" LANGUAGES C CXX ASM)
endif()

find_package(Qt6 REQUIRED COMPONENTS Core LinguistTools)
qt_standard_project_setup(REQUIRES 6.11
    I18N_SOURCE_LANGUAGE en_GB
    I18N_TRANSLATED_LANGUAGES en_US)

set(SHARED_I18N_OPTIONS
    NO_GENERATE_PLURALS_TS_FILE
    LUPDATE_OPTIONS -no-obsolete -locations none)

include_directories(source)

add_subdirectory(source/thirdparty)

# Clang's source based code coverage, run the tests with LLVM_PROFILE_FILE
# set, then use llvm-profdata and llvm-cov
option(COLLECT_COVERAGE "Build with Clang source based code coverage instrumentation" OFF)
if(COLLECT_COVERAGE)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        message(FATAL_ERROR "COLLECT_COVERAGE requires Clang")
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

option(BUILD_TESTS "Build the unit tests" OFF)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(source/tests)
endif()

set_property(DIRECTORY source/thirdparty PROPERTY QT_EXCLUDE_FROM_TRANSLATION ON)
if(BUILD_TESTS)
    set_property(DIRECTORY source/tests PROPERTY QT_EXCLUDE_FROM_TRANSLATION ON)
endif()
qt_collect_translation_source_targets(I18N_SOURCE_TARGETS)

list(APPEND I18N_TARGETS ${PROJECT_NAME})
if(NOT EMSCRIPTEN)
    list(APPEND I18N_TARGETS CrashReporter MessageBox Updater UpdateEditor)
endif()

set(I18N_QM_DIR ${CMAKE_CURRENT_BINARY_DIR}/translations)
foreach(LANGUAGE IN LISTS QT_I18N_TRANSLATED_LANGUAGES)
    set_source_files_properties(${I18N_QM_DIR}/graphia_${LANGUAGE}.qm
        PROPERTIES QT_RESOURCE_ALIAS graphia_${LANGUAGE}.qm)
endforeach()

qt_add_translations(
    TARGETS ${I18N_TARGETS}
    SOURCE_TARGETS ${I18N_SOURCE_TARGETS}
    TS_FILE_BASE graphia
    TS_FILE_DIR ${CMAKE_CURRENT_LIST_DIR}/source/translations
    QM_OUTPUT_DIRECTORY ${I18N_QM_DIR}
    RESOURCE_PREFIX /i18n
    MERGE_QT_TRANSLATIONS
    QT_TRANSLATION_CATALOGS qtbase qtdeclarative
    # There is no LRELEASE_OPTIONS -removeidentical here, because that would open
    # the possibility of a plugin being able to override an identical translation
    ${SHARED_I18N_OPTIONS})

# The following sections are only here so that the files are available
# for selection/editing in the IDE (if it supports it); nothing is
# actually built

list(APPEND MISC
    ${CMAKE_CURRENT_LIST_DIR}/scripts/defaults.ps1
    ${CMAKE_CURRENT_LIST_DIR}/scripts/defaults.sh
    ${CMAKE_CURRENT_LIST_DIR}/scripts/enter-vs-dev-shell.ps1
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
    ${CMAKE_CURRENT_LIST_DIR}/scripts/coverage.sh
    ${CMAKE_CURRENT_LIST_DIR}/scripts/cppcheck.sh
    ${CMAKE_CURRENT_LIST_DIR}/scripts/emsdk-version-for-qt-version.sh
    ${CMAKE_CURRENT_LIST_DIR}/scripts/install-llvm.sh
    ${CMAKE_CURRENT_LIST_DIR}/scripts/iwyu.sh
    ${CMAKE_CURRENT_LIST_DIR}/scripts/parse-compile_commands-json.sh
    ${CMAKE_CURRENT_LIST_DIR}/scripts/qmllint.sh
    ${CMAKE_CURRENT_LIST_DIR}/scripts/static-analysis.sh
    ${CMAKE_CURRENT_LIST_DIR}/scripts/tidy-headers.sh
    ${CMAKE_CURRENT_LIST_DIR}/scripts/upload-symbols.sh
    ${CMAKE_CURRENT_LIST_DIR}/scripts/update-thirdparty.sh
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
    ${CMAKE_CURRENT_LIST_DIR}/.github/actions/install-mesa/action.yml
    ${CMAKE_CURRENT_LIST_DIR}/.github/actions/install-qt/action.yml
    ${CMAKE_CURRENT_LIST_DIR}/.github/actions/sign/action.yml
    ${CMAKE_CURRENT_LIST_DIR}/.github/ISSUE_TEMPLATE/bug_report.md
    ${CMAKE_CURRENT_LIST_DIR}/.github/workflows/build.yml
    ${CMAKE_CURRENT_LIST_DIR}/.github/workflows/publish.yml
    ${CMAKE_CURRENT_LIST_DIR}/.github/workflows/static-analysis.yml
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
