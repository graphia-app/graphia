#include "crashhandler.h"

#include "shared/utils/thread.h"

#include <QCoreApplication>
#include <QTemporaryDir>
#include <QDir>

#include <iostream>

#if defined(Q_OS_WIN32)
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

static void launch(const wchar_t* program, const wchar_t* options, const wchar_t* dmpFile, const wchar_t* dir)
{
    static STARTUPINFO si = {0};
    static PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(si);

    static wchar_t commandLine[1024] = {0};
    swprintf(commandLine, sizeof(commandLine) / sizeof(commandLine[0]),
        L"%s %s %s %s", program, options, dmpFile, dir);

    if(!CreateProcess(nullptr, commandLine, nullptr, nullptr, FALSE,
                      CREATE_DEFAULT_ERROR_MODE,
                      nullptr, nullptr, &si, &pi))
    {
        std::cerr << "CreateProcess failed (" << GetLastError() << ")\n";
    }
}
#else
static void launch(const char* program, const char* options, const char* dmpFile, const char* dir)
{
    pid_t pid = fork();

    switch(pid)
    {
    case -1: // Failure to fork
        std::cerr << "fork() failed\n";
        exit(1);

    case 0: // Child
        execl(program, program, options, dmpFile, dir, static_cast<char*>(nullptr));

        std::cerr << "execl() failed\n";
        exit(1);

    default: // Parent
        ;
    }
}
#endif

static bool minidumpCallback(
#if defined(Q_OS_WIN32)
    const wchar_t* dumpDir, const wchar_t* minidumpId, void* context, EXCEPTION_POINTERS* ex_info, MDRawAssertionInfo*, bool success
#elif defined(Q_OS_LINUX)
    const google_breakpad::MinidumpDescriptor& md, void* context, bool success
#elif defined(Q_OS_MACOS)
    const char* dumpDir, const char* minidumpId, void* context, bool success
#endif
)
{
    // static to avoid stack allocation
    static platform_char options[1024] = {0};
    static platform_char path[1024] = {0};
    static platform_char dir[1024] = {0};

#if defined(Q_OS_WIN32)
    if(ex_info != nullptr && ex_info->ExceptionRecord != nullptr)
    {
        if(ex_info->ExceptionRecord->ExceptionCode ==
            static_cast<DWORD>(MD_EXCEPTION_CODE_WIN_UNHANDLED_CPP_EXCEPTION))
        {
            // https://support.microsoft.com/en-gb/help/185294
            // https://blogs.msdn.microsoft.com/oldnewthing/20100730-00/?p=13273/
            //FIXME: figure out the exception details and add it as an attachment or otherwise

            // Silently submit the "crash" report
            wcscat_s(options, L"-submit");
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

    std::cerr << "Handling crash...\n";

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
        exceptionHandler->userHandler()(tempDir.path());

#if defined(Q_OS_WIN32)
        tempDir.path().toWCharArray(dir);
#else
        strncpy(dir, tempDir.path().toUtf8(), sizeof(dir) - 1);
#endif
    }

#if defined(Q_OS_WIN32)
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

#ifdef Q_OS_WIN
    std::wcerr
#else
    std::cerr
#endif
        << "Starting " << exe << " " << path << " " << dir << std::endl;

    launch(exe, options, path, dir);

    // Do not pass on the exception
    return true;
}

CrashHandler::CrashHandler(const QString& crashReporterExecutableName)
{
    QString path = QDir::tempPath();

#if defined(Q_OS_WIN32)
    if(IsDebuggerPresent())
    {
        std::cerr << "Invoked by debugger, not installing CrashHandler\n";
        return;
    }

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
    // Avoid using the CrashHandler if we're started by something that's probably an IDE
    auto parentProcess = u::parentProcessName();
    if(parentProcess.contains("qtcreator", Qt::CaseInsensitive))
    {
        std::cerr << "Invoked by " << parentProcess.toStdString() <<
                     ", not installing CrashHandler\n";
        return;
    }

    strncpy(_crashReporterExecutableName,
            static_cast<const char*>(crashReporterExecutableName.toLatin1()),
            sizeof(_crashReporterExecutableName) - 1);

#if defined(Q_OS_LINUX)

    google_breakpad::MinidumpDescriptor md(path.toStdString());
    _handler = std::make_unique<google_breakpad::ExceptionHandler>(
                md, nullptr,
                minidumpCallback, this, true, -1);

#elif defined(Q_OS_MACOS)

    _handler = std::make_unique<google_breakpad::ExceptionHandler>(
                path.toStdString(), nullptr,
                minidumpCallback, this, true, nullptr);

#endif
#endif

    std::cerr << "CrashHandler installed " << _handler.get() << "\n";
}

CrashHandler::~CrashHandler() {}
