#include "namethread.h"
#include "utils.h"

#if defined(__linux__)
#include <sys/prctl.h>
#include <unistd.h>
#include <sys/syscall.h>

void nameCurrentThread(const QString& name)
{
    if(syscall(SYS_gettid) != getpid()) // Avoid renaming main thread
        prctl(PR_SET_NAME, (char*)name.toUtf8().constData());
}

const QString currentThreadName()
{
    char threadName[16] = {0};
    prctl(PR_GET_NAME, (unsigned long)threadName);

    return threadName;
}

#elif _WIN32
#include <windows.h>
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

void SetThreadName(char* threadName)
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

void nameCurrentThread(const QString& name)
{
    SetThreadName(name.toLatin1().data());
}

const QString currentThreadName()
{
    // Windows doesn't really have a concept of named threads (see above), so use the ID instead
    return QString::number(Utils::currentThreadId());
}
#else
void nameCurrentThread(const QString&) {}
const QString currentThreadName()
{
    return QString::number(Utils::currentThreadId());
}

#endif
