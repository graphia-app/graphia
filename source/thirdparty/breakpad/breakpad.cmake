include_directories(${CMAKE_CURRENT_LIST_DIR}/src)

if(NOT EMSCRIPTEN)
    list(APPEND BREAKPAD_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/src/third_party/libdisasm/ia32_implicit.c
        ${CMAKE_CURRENT_LIST_DIR}/src/third_party/libdisasm/ia32_insn.c
        ${CMAKE_CURRENT_LIST_DIR}/src/third_party/libdisasm/ia32_invariant.c
        ${CMAKE_CURRENT_LIST_DIR}/src/third_party/libdisasm/ia32_modrm.c
        ${CMAKE_CURRENT_LIST_DIR}/src/third_party/libdisasm/ia32_opcode_tables.c
        ${CMAKE_CURRENT_LIST_DIR}/src/third_party/libdisasm/ia32_operand.c
        ${CMAKE_CURRENT_LIST_DIR}/src/third_party/libdisasm/ia32_reg.c
        ${CMAKE_CURRENT_LIST_DIR}/src/third_party/libdisasm/ia32_settings.c
        ${CMAKE_CURRENT_LIST_DIR}/src/third_party/libdisasm/x86_disasm.c
        ${CMAKE_CURRENT_LIST_DIR}/src/third_party/libdisasm/x86_format.c
        ${CMAKE_CURRENT_LIST_DIR}/src/third_party/libdisasm/x86_imm.c
        ${CMAKE_CURRENT_LIST_DIR}/src/third_party/libdisasm/x86_insn.c
        ${CMAKE_CURRENT_LIST_DIR}/src/third_party/libdisasm/x86_misc.c
        ${CMAKE_CURRENT_LIST_DIR}/src/third_party/libdisasm/x86_operand_list.c

        ${CMAKE_CURRENT_LIST_DIR}/src/processor/basic_code_modules.cc
        ${CMAKE_CURRENT_LIST_DIR}/src/processor/basic_source_line_resolver.cc
        ${CMAKE_CURRENT_LIST_DIR}/src/processor/call_stack.cc
        ${CMAKE_CURRENT_LIST_DIR}/src/processor/cfi_frame_info.cc
        ${CMAKE_CURRENT_LIST_DIR}/src/processor/convert_old_arm64_context.cc
        ${CMAKE_CURRENT_LIST_DIR}/src/processor/disassembler_x86.cc
        ${CMAKE_CURRENT_LIST_DIR}/src/processor/dump_context.cc
        ${CMAKE_CURRENT_LIST_DIR}/src/processor/dump_object.cc
        ${CMAKE_CURRENT_LIST_DIR}/src/processor/exploitability.cc
        ${CMAKE_CURRENT_LIST_DIR}/src/processor/exploitability_linux.cc
        ${CMAKE_CURRENT_LIST_DIR}/src/processor/exploitability_win.cc
        ${CMAKE_CURRENT_LIST_DIR}/src/processor/fast_source_line_resolver.cc
        ${CMAKE_CURRENT_LIST_DIR}/src/processor/logging.cc
        ${CMAKE_CURRENT_LIST_DIR}/src/processor/microdump.cc
        ${CMAKE_CURRENT_LIST_DIR}/src/processor/microdump_processor.cc
        ${CMAKE_CURRENT_LIST_DIR}/src/processor/minidump.cc
        ${CMAKE_CURRENT_LIST_DIR}/src/processor/minidump_processor.cc
        ${CMAKE_CURRENT_LIST_DIR}/src/processor/module_comparer.cc
        ${CMAKE_CURRENT_LIST_DIR}/src/processor/module_serializer.cc
        ${CMAKE_CURRENT_LIST_DIR}/src/processor/pathname_stripper.cc
        ${CMAKE_CURRENT_LIST_DIR}/src/processor/process_state.cc
        ${CMAKE_CURRENT_LIST_DIR}/src/processor/proc_maps_linux.cc
        ${CMAKE_CURRENT_LIST_DIR}/src/processor/simple_symbol_supplier.cc
        ${CMAKE_CURRENT_LIST_DIR}/src/processor/source_line_resolver_base.cc
        ${CMAKE_CURRENT_LIST_DIR}/src/processor/stack_frame_cpu.cc
        ${CMAKE_CURRENT_LIST_DIR}/src/processor/stack_frame_symbolizer.cc
        ${CMAKE_CURRENT_LIST_DIR}/src/processor/stackwalk_common.cc
        ${CMAKE_CURRENT_LIST_DIR}/src/processor/stackwalker.cc
        ${CMAKE_CURRENT_LIST_DIR}/src/processor/stackwalker_amd64.cc
        ${CMAKE_CURRENT_LIST_DIR}/src/processor/stackwalker_arm.cc
        ${CMAKE_CURRENT_LIST_DIR}/src/processor/stackwalker_arm64.cc
        ${CMAKE_CURRENT_LIST_DIR}/src/processor/stackwalker_address_list.cc
        ${CMAKE_CURRENT_LIST_DIR}/src/processor/stackwalker_mips.cc
        ${CMAKE_CURRENT_LIST_DIR}/src/processor/stackwalker_ppc.cc
        ${CMAKE_CURRENT_LIST_DIR}/src/processor/stackwalker_ppc64.cc
        ${CMAKE_CURRENT_LIST_DIR}/src/processor/stackwalker_riscv.cc
        ${CMAKE_CURRENT_LIST_DIR}/src/processor/stackwalker_riscv64.cc
        ${CMAKE_CURRENT_LIST_DIR}/src/processor/stackwalker_sparc.cc
        ${CMAKE_CURRENT_LIST_DIR}/src/processor/stackwalker_x86.cc
        ${CMAKE_CURRENT_LIST_DIR}/src/processor/symbolic_constants_win.cc
        ${CMAKE_CURRENT_LIST_DIR}/src/processor/tokenize.cc
    )

    if(UNIX)
        list(APPEND BREAKPAD_SOURCES
            ${CMAKE_CURRENT_LIST_DIR}/src/client/minidump_file_writer.cc
            ${CMAKE_CURRENT_LIST_DIR}/src/common/convert_UTF.cc
            ${CMAKE_CURRENT_LIST_DIR}/src/common/md5.cc
            ${CMAKE_CURRENT_LIST_DIR}/src/common/string_conversion.cc
        )

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
        else()
            list(APPEND BREAKPAD_SOURCES
                ${CMAKE_CURRENT_LIST_DIR}/src/client/linux/crash_generation/crash_generation_client.cc
                ${CMAKE_CURRENT_LIST_DIR}/src/client/linux/crash_generation/crash_generation_server.cc
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
                ${CMAKE_CURRENT_LIST_DIR}/src/client/linux/minidump_writer/pe_file.cc
                ${CMAKE_CURRENT_LIST_DIR}/src/common/linux/breakpad_getcontext.S
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

    if(MSVC)
        list(APPEND BREAKPAD_SOURCES
            ${CMAKE_CURRENT_LIST_DIR}/src/client/windows/crash_generation/crash_generation_client.cc
            ${CMAKE_CURRENT_LIST_DIR}/src/common/windows/guid_string.cc
            ${CMAKE_CURRENT_LIST_DIR}/src/client/windows/handler/exception_handler.cc
        )
    endif()
endif()

list(APPEND BREAKPAD_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/exceptionrecord.h
    ${CMAKE_CURRENT_LIST_DIR}/crashhandler.h
)

list(APPEND BREAKPAD_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/crashhandler.cpp
)

list(APPEND STATIC_THIRDPARTY_HEADERS ${BREAKPAD_HEADERS})
list(APPEND STATIC_THIRDPARTY_SOURCES
    ${BREAKPAD_SOURCES}
)
