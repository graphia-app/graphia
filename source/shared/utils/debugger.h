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

#ifndef DEBUGGER_H
#define DEBUGGER_H

#include "shared/utils/thread.h"

#include <QString>
#include <QtGlobal>

#include <vector>
#include <algorithm>

#if defined(Q_OS_WIN)
#include <Windows.h>
#elif defined(Q_OS_MACOS)
#include <array>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <unistd.h>
#elif defined(Q_OS_LINUX)
#include <QFile>
#include <QTextStream>
#endif

namespace u
{
inline bool isBeingTraced()
{
#if defined(Q_OS_LINUX)
    QFile statusFile(u"/proc/self/status"_s);
    if(!statusFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    const QString tracerPidKey = u"TracerPid:"_s;
    QTextStream ts(&statusFile);
    QString line;
    while(ts.readLineInto(&line))
    {
        if(line.startsWith(tracerPidKey))
            return line.mid(tracerPidKey.size()).trimmed().toInt() != 0;
    }

    return false;
#elif defined(Q_OS_MACOS)
    std::array<int, 4> mib = {CTL_KERN, KERN_PROC, KERN_PROC_PID, getpid()};
    struct kinfo_proc info = {};
    size_t size = sizeof(info);

    if(sysctl(mib.data(), static_cast<u_int>(mib.size()), &info, &size, nullptr, 0) != 0)
        return false;

    return (info.kp_proc.p_flag & P_TRACED) != 0;
#else
    return false;
#endif
}

inline bool isDebuggerPresent()
{
#if defined(Q_OS_WIN)
    return static_cast<bool>(IsDebuggerPresent());
#else
    if(isBeingTraced())
        return true;

    auto parentProcessName = u::parentProcessName();
    if(!parentProcessName.isEmpty())
    {
        std::vector<QString> debuggers = {"qtcreator", "gdb", "debugserver"};

        return std::ranges::any_of(debuggers, [&parentProcessName](const auto& debugger)
        {
            return parentProcessName.contains(debugger, Qt::CaseInsensitive);
        });
    }

    return false;
#endif
}

} // namespace u

#endif // DEBUGGER_H
