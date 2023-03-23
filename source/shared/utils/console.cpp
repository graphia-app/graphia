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

#include "console.h"

#include <QtGlobal>

#ifdef Q_OS_WIN
#include <Windows.h>
#include <io.h>
#else
#include <unistd.h>
#endif

bool isRunningInConsole()
{
#ifdef Q_OS_WIN
    if(_isatty(_fileno(stderr)) != 0)
#else
    if(isatty(fileno(stderr)) != 0)
#endif
        return true;

    return false;
}

DWORD enableConsoleMode()
{
    DWORD originalMode = 0;

#ifdef Q_OS_WIN
    if(!isRunningInConsole())
        return originalMode;

    auto hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    if(hStdout != INVALID_HANDLE_VALUE)
    {
        GetConsoleMode(hStdout, &originalMode) && SetConsoleMode(hStdout,
            originalMode|
            ENABLE_VIRTUAL_TERMINAL_PROCESSING|
            DISABLE_NEWLINE_AUTO_RETURN);
    }
#endif

    return originalMode;
}

void restoreConsoleMode(DWORD mode)
{
#ifdef Q_OS_WIN
    if(!isRunningInConsole())
        return;

    auto hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    if(hStdout != INVALID_HANDLE_VALUE)
        SetConsoleMode(hStdout, mode);
#else
    Q_UNUSED(mode);
#endif
}
