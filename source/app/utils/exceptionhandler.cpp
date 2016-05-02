#include "exceptionhandler.h"

#include <QDir>
#include <QCoreApplication>

#include <iostream>

#include <unistd.h>

#if defined(Q_OS_MAC)
#include "../thirdparty/breakpad/src/client/mac/handler/exception_handler.h"
#elif defined(Q_OS_LINUX)
#include "../thirdparty/breakpad/src/client/linux/handler/exception_handler.h"
#elif defined(Q_OS_WIN32)
#include "../thirdparty/breakpad/src/client/windows/handler/exception_handler.h"
#endif

#ifdef Q_OS_WIN
static void launch(wchar_t* program, wchar_t* path)
{
    static STARTUPINFO si = {0};
    static PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(si);

    static wchar_t commandLine[1024] = {0};
    wcsncat(commandLine, program, sizeof(commandLine) - 1);
    wcsncat(commandLine, L" ", sizeof(commandLine) - 1);
    wcsncat(commandLine, path, sizeof(commandLine) - 1);

    if(!CreateProcess(NULL, commandLine, 0, FALSE, 0,
                      CREATE_DEFAULT_ERROR_MODE|CREATE_NO_WINDOW|DETACHED_PROCESS,
                      0, 0, &si, &pi))
    {
        std::cerr << "CreateProcess failed (" << GetLastError() << ")\n";
        return;
    }

    //FIXME huh?
    /*if(PostThreadMessage(pi.dwThreadId, WM_QUIT, 0, 0))
        std::cout << "Request to terminate process has been sent!";

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);*/
}
#else
static void launch(const char* program, const char* path)
{
    pid_t pid = fork();

    switch(pid)
    {
    case -1: // Failure to fork
        std::cerr << "fork() failed\n";
        exit(1);

    case 0: // Child
        execl(program, program, path, static_cast<char*>(nullptr));

        std::cerr << "execl() failed\n";
        exit(1);

    default: // Parent
        ;
    }
}
#endif

static bool minidumpCallback(
#if defined(Q_OS_WIN32)
    const wchar_t* dumpDir, const wchar_t* minidumpId, void* context, EXCEPTION_POINTERS*, MDRawAssertionInfo*, bool success
#elif defined(Q_OS_LINUX)
    const google_breakpad::MinidumpDescriptor& md, void* context, bool success
#elif defined(Q_OS_MAC)
    const char* dumpDir, const char* minidumpId, void* context, bool success
#endif
)
{
    if(!success)
    {
        std::cerr << "Failed to generate minidump\n";
        return false;
    }

    ExceptionHandler* exceptionHandler = reinterpret_cast<ExceptionHandler*>(context);
    const auto* exe = exceptionHandler->crashReporterExecutableName();

    // static to avoid stack allocation
    static platform_char path[1024] = {0};

#if defined(Q_OS_WIN32)
    wcsncat(path, dumpDir, sizeof(path) - 1);
    wcsncat(path, L"\\", sizeof(path) - 1);
    wcsncat(path, minidumpId, sizeof(path) - 1);
    wcsncat(path, L".dmp", sizeof(path) - 1);
#elif defined(Q_OS_LINUX)
    strncpy(path, md.path(), sizeof(path) - 1);
#elif defined(Q_OS_MAC)
    strncat(path, dumpDir, sizeof(path) - 1);
    strncat(path, "/", sizeof(path) - 1);
    strncat(path, minidumpId, sizeof(path) - 1);
    strncat(path, ".dmp", sizeof(path) - 1);
#endif

    launch(exe, path);

    return false;
}

ExceptionHandler::ExceptionHandler()
{
    QString path = QDir::tempPath();

    QString crashReporterExecutableName(
                QCoreApplication::applicationDirPath() +
                QDir::separator() +
                "CrashReporter");

#if defined(Q_OS_WIN32)
    crashReporterExecutableName.append(".exe");
    wcsncpy(_crashReporterExecutableName,
            static_cast<const wchar_t*>(crashReporterExecutableName.utf16()),
            sizeof(_crashReporterExecutableName) - 1);

    _handler = std::make_unique<google_breakpad::ExceptionHandler>(
                static_cast<const wchar_t*>(path.utf16()), nullptr,
                minidumpCallback, this, true);
#else
    strncpy(_crashReporterExecutableName,
            static_cast<const char*>(crashReporterExecutableName.toLatin1()),
            sizeof(_crashReporterExecutableName) - 1);

#if defined(Q_OS_LINUX)

    google_breakpad::MinidumpDescriptor md(path.toStdString());
    _handler = std::make_unique<google_breakpad::ExceptionHandler>(
                md, nullptr,
                minidumpCallback, this, true, -1);

#elif defined(Q_OS_MAC)
    _handler = std::make_unique<google_breakpad::ExceptionHandler>(
                path.toStdString(), nullptr,
                minidumpCallback, this, true, nullptr);
#endif
#endif
}

ExceptionHandler::~ExceptionHandler() {}
