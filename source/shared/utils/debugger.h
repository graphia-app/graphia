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

#include <QtGlobal>

#include <vector>
#include <algorithm>

#if defined(Q_OS_WIN)
#include <Windows.h>
#endif

namespace u
{
inline bool isDebuggerPresent()
{
#if defined(Q_OS_WIN)
    return static_cast<bool>(IsDebuggerPresent());
#else
    //FIXME on Linux we could probably use one of the solutions here:
    // https://stackoverflow.com/questions/3596781/how-to-detect-if-the-current-process-is-being-run-by-gdb

    auto parentProcessName = u::parentProcessName();
    if(!parentProcessName.isEmpty())
    {
        std::vector<QString> debuggers = {"qtcreator", "gdb", "debugserver"};

        return std::any_of(debuggers.begin(), debuggers.end(),
                           [&parentProcessName](const auto& debugger)
        {
            return parentProcessName.contains(debugger, Qt::CaseInsensitive);
        });
    }

    return false;
#endif
}

} // namespace u

#endif // DEBUGGER_H
