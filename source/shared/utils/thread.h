/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

#ifndef THREAD_H
#define THREAD_H

#include <thread>

#include <QTextStream>
#include <QString>

using namespace Qt::Literals::StringLiterals;

namespace u
{
    inline int currentThreadId();
    inline void setCurrentThreadName(const QString& name);
    inline QString currentThreadName();
    inline QString parentProcessName();
} // namespace u

inline int u::currentThreadId()
{
    return static_cast<int>(std::hash<std::thread::id>()(std::this_thread::get_id()));
}

#if defined(__APPLE__)

#include <sys/types.h>
#include <sys/proc_info.h>
#include <libproc.h>
#include <unistd.h>

void u::setCurrentThreadName(const QString& name)
{
    pthread_setname_np(static_cast<const char*>(name.toUtf8().constData()));
}

QString u::currentThreadName()
{
    char threadName[16] = {0};
    pthread_getname_np(pthread_self(), threadName, sizeof(threadName));

    return {threadName};
}

QString u::parentProcessName()
{
    auto parentPid = getppid();

    struct proc_bsdinfo proc;
    if(proc_pidinfo(parentPid, PROC_PIDTBSDINFO, 0, &proc, PROC_PIDTBSDINFO_SIZE) == PROC_PIDTBSDINFO_SIZE)
        return {proc.pbi_name};

    return {};
}
#elif defined(__linux__)
#include <sys/prctl.h>
#include <unistd.h>
#include <sys/syscall.h>

#include <QFile>

void u::setCurrentThreadName(const QString& name)
{
    prctl(PR_SET_NAME, static_cast<const char*>(name.toUtf8().constData()));
}

QString u::currentThreadName()
{
    char threadName[16] = {0};
    prctl(PR_GET_NAME, reinterpret_cast<uint64_t>(threadName));

    return {threadName};
}

QString u::parentProcessName()
{
    auto ppid = getppid();

    QFile procFile(u"/proc/%1/cmdline"_s.arg(ppid));
    if(procFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream ts(&procFile);
        const QString commandLine = ts.readLine();
        auto tokens = commandLine.split(QChar(0));

        if(!tokens.empty())
            return tokens.at(0);
    }

    return {};
}

#elif defined(_WIN32) && !defined(__MINGW32__)
#include "shared/utils/msvcwarningsuppress.h"

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
    MSVC_WARNING_SUPPRESS_NEXTLINE(6320)
    __except(EXCEPTION_EXECUTE_HANDLER)
    MSVC_WARNING_SUPPRESS_NEXTLINE(6322)
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

// Sigh... windows.h
#undef min
#undef max
#elif defined(EMSCRIPTEN)
#include <pthread.h>
#include <emscripten/threading.h>

#include <map>

inline std::map<pthread_t, QString> emscriptenThreadNames;

inline void u::setCurrentThreadName(const QString& name)
{
    // This is apparently only applicable when --threadprofiler is added to the linker flags...
    emscripten_set_thread_name(pthread_self(), static_cast<const char*>(name.toUtf8().constData()));

    // ...otherwise we just track our own thread names
    emscriptenThreadNames[pthread_self()] = name;
}

inline QString u::currentThreadName()
{
    if(emscriptenThreadNames.contains(pthread_self()))
        return emscriptenThreadNames.at(pthread_self());

    if(emscripten_is_main_runtime_thread() == 0)
        return QString::number(u::currentThreadId());

    return {};
}

inline QString u::parentProcessName() { return {}; }
#else
inline void u::setCurrentThreadName(const QString&) {}

inline QString u::currentThreadName()
{
    return QString::number(u::currentThreadId());
}

inline QString u::parentProcessName() { return {}; }
#endif

#endif // THREAD_H
