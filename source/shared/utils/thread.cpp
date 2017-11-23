#include "thread.h"

#include <thread>

#include <QTextStream>

int u::currentThreadId()
{
    return static_cast<int>(std::hash<std::thread::id>()(std::this_thread::get_id()));
}

#if defined(__linux__)
#include <sys/prctl.h>
#include <unistd.h>
#include <sys/syscall.h>

#include <QFile>

void u::setCurrentThreadName(const QString& name)
{
    if(syscall(SYS_gettid) != getpid()) // Avoid renaming main thread
        prctl(PR_SET_NAME, static_cast<const char*>(name.toUtf8().constData()));
}

QString u::currentThreadName()
{
    char threadName[16] = {0};
    prctl(PR_GET_NAME, reinterpret_cast<uint64_t>(threadName)); // NOLINT

    return QString(threadName);
}

QString u::parentProcessName()
{
    auto ppid = getppid();

    QFile procFile(QStringLiteral("/proc/%1/cmdline").arg(ppid));
    if(procFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream ts(&procFile);
        QString commandLine = ts.readLine();
        auto tokens = commandLine.split(QRegExp(QString(QChar(0))));

        if(!tokens.empty())
            return tokens.at(0);
    }

    return {};
}

#elif defined(_WIN32) && !defined(__MINGW32__)
#include <windows.h>
#include <tlhelp32.h>
#include <Psapi.h>

const DWORD MS_VC_EXCEPTION  =0x406D1388;

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
    DWORD dwType; // Must be 0x1000.
    LPCSTR szName; // Pointer to name (in user addr space).
    DWORD dwThreadID; // Thread ID (-1=caller thread).
    DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

static void SetThreadName(char* threadName)
{
    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = threadName;
    info.dwThreadID = -1;
    info.dwFlags = 0;

    __try
    {
        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }
}

void u::setCurrentThreadName(const QString& name)
{
    SetThreadName(name.toLatin1().data());
}

QString u::currentThreadName()
{
    // Windows doesn't really have a concept of named threads (see above), so use the ID instead
    return QString::number(u::currentThreadId());
}

QString u::parentProcessName()
{
    PROCESSENTRY32 pe = {0};
    DWORD ppid = 0;
    pe.dwSize = sizeof(PROCESSENTRY32);
    HANDLE handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if(Process32First(handle, &pe))
    {
        do
        {
            if(pe.th32ProcessID == GetCurrentProcessId())
            {
                ppid = pe.th32ParentProcessID;
                break;
            }
        } while(Process32Next(handle, &pe));
    }
    CloseHandle(handle);

    HANDLE processHandle = OpenProcess(
        PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
        FALSE,
        ppid);

    if(processHandle)
    {
        TCHAR charBuffer[MAX_PATH];
        if(GetModuleFileNameEx(processHandle, 0, charBuffer, MAX_PATH))
            return QString::fromWCharArray(charBuffer);

        CloseHandle(processHandle);
    }

    return {};
}
#else
void u::setCurrentThreadName(const QString&) {}
QString u::currentThreadName()
{
    return QString::number(u::currentThreadId());
}
QString u::parentProcessName() { return {}; }
#endif
