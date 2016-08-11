INCLUDEPATH += breakpad/src

unix {
    SOURCES += \
        breakpad/src/client/minidump_file_writer.cc \
        breakpad/src/common/convert_UTF.c \
        breakpad/src/common/md5.cc \
        breakpad/src/common/string_conversion.cc
}

linux {
    SOURCES += \
        breakpad/src/client/linux/crash_generation/crash_generation_client.cc \
        breakpad/src/client/linux/dump_writer_common/thread_info.cc \
        breakpad/src/client/linux/dump_writer_common/ucontext_reader.cc \
        breakpad/src/client/linux/handler/exception_handler.cc \
        breakpad/src/client/linux/handler/minidump_descriptor.cc \
        breakpad/src/client/linux/log/log.cc \
        breakpad/src/client/linux/microdump_writer/microdump_writer.cc \
        breakpad/src/client/linux/minidump_writer/linux_core_dumper.cc \
        breakpad/src/client/linux/minidump_writer/linux_dumper.cc \
        breakpad/src/client/linux/minidump_writer/linux_ptrace_dumper.cc \
        breakpad/src/client/linux/minidump_writer/minidump_writer.cc \
        breakpad/src/common/linux/elf_core_dump.cc \
        breakpad/src/common/linux/elfutils.cc \
        breakpad/src/common/linux/file_id.cc \
        breakpad/src/common/linux/guid_creator.cc \
        breakpad/src/common/linux/linux_libc_support.cc \
        breakpad/src/common/linux/memory_mapped_file.cc \
        breakpad/src/common/linux/safe_readlink.cc
}

mac {
    SOURCES += \
        breakpad/src/client/mac/handler/exception_handler.cc \
        breakpad/src/client/mac/handler/minidump_generator.cc \
        breakpad/src/client/mac/handler/dynamic_images.cc \
        breakpad/src/client/mac/crash_generation/crash_generation_client.cc \
        breakpad/src/client/mac/handler/breakpad_nlist_64.cc \
        breakpad/src/common/mac/file_id.cc \
        breakpad/src/common/mac/macho_utilities.cc \
        breakpad/src/common/mac/macho_id.cc \
        breakpad/src/common/mac/string_utilities.cc \
        breakpad/src/common/mac/bootstrap_compat.cc \
        breakpad/src/common/mac/macho_walker.cc

    OBJECTIVE_SOURCES += \
        breakpad/src/common/mac/MachIPC.mm

    LIBS += -framework CoreFoundation
}

win32 {
    SOURCES += \
        breakpad/src/client/windows/crash_generation/crash_generation_client.cc \
        breakpad/src/common/windows/guid_string.cc \
        breakpad/src/client/windows/handler/exception_handler.cc
}

HEADERS += breakpad/crashhandler.h
SOURCES += breakpad/crashhandler.cpp
