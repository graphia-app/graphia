/* Copyright © 2013-2023 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "crashhandler.h"
#include "exceptionrecord.h"

#include "shared/utils/thread.h"
#include "shared/utils/debugger.h"

#include <QCoreApplication>
#include <QTemporaryDir>
#include <QDir>

#include <iostream>

#if defined(Q_OS_WIN)
#include <breakpad/src/client/windows/handler/exception_handler.h>
#else

#include <unistd.h>

#if defined(Q_OS_MACOS)
#include <breakpad/src/client/mac/handler/exception_handler.h>
#elif defined(Q_OS_LINUX)
#include <breakpad/src/client/linux/handler/exception_handler.h>
#endif

#endif

#ifdef Q_OS_WIN

#include <cwchar>

static void launch(const wchar_t* program, const wchar_t** options, const wchar_t* dmpFile, const wchar_t* dir)
{
    static STARTUPINFO si = {0};
    static PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(si);

    static wchar_t commandLine[1024] = {0};
    wcscat_s(commandLine, program);
    wcscat_s(commandLine, L" ");
    while(*options)
    {
        wcscat_s(commandLine, *options);
        wcscat_s(commandLine, L" ");
        options++;
    }
    wcscat_s(commandLine, dmpFile);

    if(dir && *dir)
    {
        wcscat_s(commandLine, L" ");
        wcscat_s(commandLine, dir);
    }

    if(!CreateProcess(nullptr, commandLine, nullptr, nullptr, FALSE,
        CREATE_DEFAULT_ERROR_MODE, nullptr, nullptr, &si, &pi))
    {
        std::cerr << "CreateProcess failed (" << GetLastError() << ")\n";
    }
}
#else
static void launch(const char* program, const char** options, const char* dmpFile, const char* dir)
{
    pid_t pid = fork();

    switch(pid)
    {
    case -1: // Failure to fork
        std::cerr << "fork() failed\n";
        exit(1);

    case 0: // Child
    {
        const platform_char* args[64] = {0};
        int i = 0;

        auto addArg = [&](const auto* arg)
        {
            if(i < (sizeof(args) / sizeof(args[0])))
                args[i++] = arg;
        };

        addArg(program);

        while(*options)
        {
            addArg(*options);
            options++;
        }

        addArg(dmpFile);

        if(dir && *dir)
            addArg(dir);

        execv(program, const_cast<platform_char* const*>(args));

        std::cerr << "execv() failed\n";
        exit(1);
    }

    default: // Parent
        ;
    }
}
#endif

static bool minidumpCallback(
#if defined(Q_OS_WIN)
    const wchar_t* dumpDir, const wchar_t* minidumpId, void* context, EXCEPTION_POINTERS* ex_info, MDRawAssertionInfo*, bool success
#elif defined(Q_OS_LINUX)
    const google_breakpad::MinidumpDescriptor& md, void* context, bool success
#elif defined(Q_OS_MACOS)
    const char* dumpDir, const char* minidumpId, void* context, bool success
#endif
)
{
    // static to avoid stack allocation
    static platform_char path[1024] = {0};
    static platform_char dir[1024] = {0};

    static platform_char options[1024] = {0};
    static platform_char* optionsArray[32] = {0};

    auto addOption = [&, p = &options[0], i = 0](const platform_char* option) mutable
    {
        const auto used = p - options;
        const auto optionsSize = sizeof(options) / sizeof(options[0]);
        const auto remaining = optionsSize - used;

        if(used >= optionsSize)
            return;

        *p = '\0';

#ifdef Q_OS_WIN
        wcscat_s(p, remaining, option);
        optionsArray[i++] = p;
        p += wcslen(p) + 1;
#else
        strncat(p, option, remaining - 1);
        optionsArray[i++] = p;
        p += strlen(p) + 1;
#endif
    };

    std::cerr << "Handling crash...\n";

#if defined(Q_OS_WIN)
    if(ex_info != nullptr && ex_info->ExceptionRecord != nullptr)
    {
        if(ex_info->ExceptionRecord->ExceptionCode ==
            static_cast<DWORD>(MD_EXCEPTION_CODE_WIN_UNHANDLED_CPP_EXCEPTION))
        {
            // https://support.microsoft.com/en-gb/help/185294
            // https://blogs.msdn.microsoft.com/oldnewthing/20100730-00/?p=13273/

            auto exceptionTypeName = exceptionRecordType(ex_info->ExceptionRecord);
            static wchar_t exceptionTypeName_wchar[1024];
            if(exceptionTypeName != nullptr && mbstowcs_s(nullptr, exceptionTypeName_wchar,
                exceptionTypeName, strlen(exceptionTypeName)) == 0)
            {
                addOption(L"-submit");
                addOption(L"-description");

                static wchar_t description[1024] = {0};
                wcscat_s(description, L"\"");
                wcscat_s(description, exceptionTypeName_wchar);
                wcscat_s(description, L"\"");
                addOption(description);
            }
            else
            {
                // If we weren't able to determine the exception type,
                // we still want to silently submit
                addOption(L"-submit");
            }
        }
        else
        {
            // Occasionally we see exceptions that aren't really crashes at all
            // Instead of handling and reporting these, we just pass them on
            static const DWORD PASS_ON_EXCEPTIONS[] =
            {
                RPC_S_SERVER_UNAVAILABLE
            };

            static const size_t NUM_PASS_ON_EXCEPTIONS = sizeof(PASS_ON_EXCEPTIONS) /
                sizeof(PASS_ON_EXCEPTIONS[0]);

            const auto thrownCode = HRESULT_CODE(ex_info->ExceptionRecord->ExceptionCode);
            for(size_t i = 0; i < NUM_PASS_ON_EXCEPTIONS; i++)
            {
                const auto code = PASS_ON_EXCEPTIONS[i];
                if(thrownCode == code)
                {
                    std::cerr << "Caught HRESULT " << thrownCode << ", ignoring and passing on...\n";
                    return false;
                }
            }
        }
    }
#endif

    if(!success)
    {
        std::cerr << "Failed to generate minidump\n";
        return false;
    }

    CrashHandler* exceptionHandler = reinterpret_cast<CrashHandler*>(context);
    const auto* exe = exceptionHandler->crashReporterExecutableName();

    if(exceptionHandler->userHandler() != nullptr)
    {
        QTemporaryDir tempDir;
        tempDir.setAutoRemove(false);

        std::cerr << "Invoking userHandler\n";
        exceptionHandler->userHandler()(tempDir.path());

#if defined(Q_OS_WIN)
        tempDir.path().toWCharArray(dir);
#else
        strncpy(dir, tempDir.path().toUtf8(), sizeof(dir) - 1);
#endif
    }
    else
        std::cerr << "No userHandler installer\n";

#if defined(Q_OS_WIN)
    swprintf(path, sizeof(path) / sizeof(path[0]),
        L"%s\\%s.dmp", dumpDir, minidumpId);
#elif defined(Q_OS_LINUX)
    strncpy(path, md.path(), sizeof(path) - 1);
#elif defined(Q_OS_MACOS)
    strncat(path, dumpDir, sizeof(path) - 1);
    strncat(path, "/", sizeof(path) - 1);
    strncat(path, minidumpId, sizeof(path) - 1);
    strncat(path, ".dmp", sizeof(path) - 1);
#endif

    if(!exceptionHandler->reason().isEmpty())
    {
#ifdef Q_OS_WIN
        addOption(L"-submit");
        addOption(L"-description");

        const size_t reasonBufferSize = 1024;
        static wchar_t reason[reasonBufferSize] = {0};
        wcscat_s(reason, L"\"");
        if(exceptionHandler->reason().length() < (reasonBufferSize - 3/*2 quotes, 1 nul*/))
            exceptionHandler->reason().toWCharArray(&reason[1]);
        wcscat_s(reason, L"\"");

        addOption(reason);
#else
        addOption("-submit");
        addOption("-description");
        auto reasonByteArray = exceptionHandler->reason().toUtf8();
        addOption(reasonByteArray.data());
#endif
    }

#ifdef Q_OS_WIN
    std::wcerr
#else
    std::cerr
#endif
        << "Starting " << exe << " " << path << " " << dir << std::endl;

    launch(exe, const_cast<const platform_char**>(&optionsArray[0]), path, dir);

    std::cerr << "Successfully handled crash\n";

    // Do not pass on the exception
    return true;
}

CrashHandler::CrashHandler(const QString& crashReporterExecutableName)
{
    QString path = QDir::tempPath();

    if(u::isDebuggerPresent())
    {
        std::cerr << "Invoked by debugger, not installing CrashHandler\n";
        return;
    }

#if defined(Q_OS_WIN)
    int length = crashReporterExecutableName.toWCharArray(_crashReporterExecutableName);
    _crashReporterExecutableName[length] = 0;
    wchar_t tempPath[1024] = {0};
    length = path.toWCharArray(tempPath);
    tempPath[length] = 0;

    // Prevent the default Windows handler kicking in
    SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);

    _handler = std::make_unique<google_breakpad::ExceptionHandler>(
                tempPath, nullptr,
                minidumpCallback, this,
                google_breakpad::ExceptionHandler::HANDLER_EXCEPTION);
#else
    strncpy(_crashReporterExecutableName,
        static_cast<const char*>(crashReporterExecutableName.toLatin1()),
        sizeof(_crashReporterExecutableName) - 1);

#if defined(Q_OS_LINUX)

    google_breakpad::MinidumpDescriptor md(path.toStdString());
    _handler = std::make_unique<google_breakpad::ExceptionHandler>(
        md, nullptr, minidumpCallback, this, true, -1);

#elif defined(Q_OS_MACOS)

    _handler = std::make_unique<google_breakpad::ExceptionHandler>(
        path.toStdString(), nullptr, minidumpCallback, this, true, nullptr);

#endif
#endif

    std::cerr << "CrashHandler installed " << _handler.get() << "\n";
}

CrashHandler::~CrashHandler() {}

void CrashHandler::submitMinidump(const QString& reason)
{
    if(_handler == nullptr)
        return;

    _reason = reason;
    _handler->WriteMinidump();
    _reason.clear();
}
