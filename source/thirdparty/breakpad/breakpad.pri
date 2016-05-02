INCLUDEPATH += ../thirdparty/breakpad/src

unix {
    SOURCES += \
        ../thirdparty/breakpad/src/client/linux/crash_generation/crash_generation_client.cc \
        ../thirdparty/breakpad/src/client/linux/crash_generation/crash_generation_server.cc \
        ../thirdparty/breakpad/src/client/linux/dump_writer_common/thread_info.cc \
        ../thirdparty/breakpad/src/client/linux/dump_writer_common/ucontext_reader.cc \
        ../thirdparty/breakpad/src/client/linux/handler/exception_handler.cc \
        ../thirdparty/breakpad/src/client/linux/handler/minidump_descriptor.cc \
        ../thirdparty/breakpad/src/client/linux/log/log.cc \
        ../thirdparty/breakpad/src/client/linux/microdump_writer/microdump_writer.cc \
        ../thirdparty/breakpad/src/client/linux/minidump_writer/linux_core_dumper.cc \
        ../thirdparty/breakpad/src/client/linux/minidump_writer/linux_dumper.cc \
        ../thirdparty/breakpad/src/client/linux/minidump_writer/linux_ptrace_dumper.cc \
        ../thirdparty/breakpad/src/client/linux/minidump_writer/minidump_writer.cc \
        ../thirdparty/breakpad/src/client/minidump_file_writer.cc \
        ../thirdparty/breakpad/src/common/convert_UTF.c \
        ../thirdparty/breakpad/src/common/md5.cc \
        ../thirdparty/breakpad/src/common/string_conversion.cc \
        ../thirdparty/breakpad/src/common/linux/elf_core_dump.cc \
        ../thirdparty/breakpad/src/common/linux/elfutils.cc \
        ../thirdparty/breakpad/src/common/linux/file_id.cc \
        ../thirdparty/breakpad/src/common/linux/guid_creator.cc \
        ../thirdparty/breakpad/src/common/linux/linux_libc_support.cc \
        ../thirdparty/breakpad/src/common/linux/memory_mapped_file.cc \
        ../thirdparty/breakpad/src/common/linux/safe_readlink.cc
}
