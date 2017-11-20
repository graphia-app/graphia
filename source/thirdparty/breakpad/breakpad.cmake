include(${CMAKE_CURRENT_SOURCE_DIR}/../unity.cmake)

include_directories(${CMAKE_CURRENT_LIST_DIR}/src)

if(UNIX)
    list(APPEND BREAKPAD_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/src/client/minidump_file_writer.cc
        ${CMAKE_CURRENT_LIST_DIR}/src/common/convert_UTF.c
        ${CMAKE_CURRENT_LIST_DIR}/src/common/md5.cc
        ${CMAKE_CURRENT_LIST_DIR}/src/common/string_conversion.cc
    )

    if(NOT APPLE)
        list(APPEND BREAKPAD_SOURCES
            ${CMAKE_CURRENT_LIST_DIR}/src/client/linux/crash_generation/crash_generation_client.cc
            ${CMAKE_CURRENT_LIST_DIR}/src/client/linux/dump_writer_common/thread_info.cc
            ${CMAKE_CURRENT_LIST_DIR}/src/client/linux/dump_writer_common/ucontext_reader.cc
            ${CMAKE_CURRENT_LIST_DIR}/src/client/linux/handler/exception_handler.cc
            ${CMAKE_CURRENT_LIST_DIR}/src/client/linux/handler/minidump_descriptor.cc
            ${CMAKE_CURRENT_LIST_DIR}/src/client/linux/log/log.cc
            ${CMAKE_CURRENT_LIST_DIR}/src/client/linux/microdump_writer/microdump_writer.cc
            ${CMAKE_CURRENT_LIST_DIR}/src/client/linux/minidump_writer/linux_core_dumper.cc
            ${CMAKE_CURRENT_LIST_DIR}/src/client/linux/minidump_writer/linux_dumper.cc
            ${CMAKE_CURRENT_LIST_DIR}/src/client/linux/minidump_writer/linux_ptrace_dumper.cc
            ${CMAKE_CURRENT_LIST_DIR}/src/client/linux/minidump_writer/minidump_writer.cc
            ${CMAKE_CURRENT_LIST_DIR}/src/common/linux/elf_core_dump.cc
            ${CMAKE_CURRENT_LIST_DIR}/src/common/linux/elfutils.cc
            ${CMAKE_CURRENT_LIST_DIR}/src/common/linux/file_id.cc
            ${CMAKE_CURRENT_LIST_DIR}/src/common/linux/guid_creator.cc
            ${CMAKE_CURRENT_LIST_DIR}/src/common/linux/linux_libc_support.cc
            ${CMAKE_CURRENT_LIST_DIR}/src/common/linux/memory_mapped_file.cc
            ${CMAKE_CURRENT_LIST_DIR}/src/common/linux/safe_readlink.cc
        )
    endif()
endif()

if(APPLE)
    list(APPEND BREAKPAD_SOURCES
            ${CMAKE_CURRENT_LIST_DIR}/src/client/mac/handler/exception_handler.cc
            ${CMAKE_CURRENT_LIST_DIR}/src/client/mac/handler/minidump_generator.cc
            ${CMAKE_CURRENT_LIST_DIR}/src/client/mac/handler/dynamic_images.cc
            ${CMAKE_CURRENT_LIST_DIR}/src/client/mac/crash_generation/crash_generation_client.cc
            ${CMAKE_CURRENT_LIST_DIR}/src/client/mac/handler/breakpad_nlist_64.cc
            ${CMAKE_CURRENT_LIST_DIR}/src/common/mac/file_id.cc
            ${CMAKE_CURRENT_LIST_DIR}/src/common/mac/macho_utilities.cc
            ${CMAKE_CURRENT_LIST_DIR}/src/common/mac/macho_id.cc
            ${CMAKE_CURRENT_LIST_DIR}/src/common/mac/string_utilities.cc
            ${CMAKE_CURRENT_LIST_DIR}/src/common/mac/bootstrap_compat.cc
            ${CMAKE_CURRENT_LIST_DIR}/src/common/mac/macho_walker.cc
            ${CMAKE_CURRENT_LIST_DIR}/src/common/mac/MachIPC.mm
    )
endif()

if(MSVC)
    list(APPEND BREAKPAD_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/src/client/windows/crash_generation/crash_generation_client.cc
        ${CMAKE_CURRENT_LIST_DIR}/src/common/windows/guid_string.cc
        ${CMAKE_CURRENT_LIST_DIR}/src/client/windows/handler/exception_handler.cc
    )
endif()

list(APPEND HEADERS ${CMAKE_CURRENT_LIST_DIR}/crashhandler.h)
list(APPEND BREAKPAD_SOURCES ${CMAKE_CURRENT_LIST_DIR}/crashhandler.cpp)

if(UNITY_BUILD)
    GenerateUnity(ORIGINAL_SOURCES BREAKPAD_SOURCES UNITY_PREFIX "breakpad")
endif()

list(APPEND SOURCES ${BREAKPAD_SOURCES})
