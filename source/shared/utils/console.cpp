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

#include <iostream>
#else
#include <unistd.h>
#include <sys/ioctl.h>
#include <cstdio>
#endif

#ifdef Q_OS_WIN
static bool runningInConsole = false;
#endif

bool isRunningInConsole()
{
#ifdef Q_OS_WIN
    return runningInConsole;
#else
    return isatty(fileno(stderr)) != 0;
#endif
}

DWORD enableConsoleMode()
{
#ifdef Q_OS_WIN
    DWORD originalMode = 0;

    if(AttachConsole(ATTACH_PARENT_PROCESS))
    {
        runningInConsole = true;

        STARTUPINFO startupInfo = {};
        GetStartupInfo(&startupInfo);

        if((startupInfo.dwFlags & STARTF_USESTDHANDLES) == 0)
        {
            FILE* stdoutStream; freopen_s(&stdoutStream, "CONOUT$", "w", stdout);
            FILE* stderrStream; freopen_s(&stderrStream, "CONOUT$", "w", stderr);
            FILE* stdinStream;  freopen_s(&stdinStream,  "CONIN$",  "r", stdin);

            std::cout.clear(); std::wcout.clear();
            std::cerr.clear(); std::wcerr.clear();
            std::cin.clear();  std::wcin.clear();
        }

        auto hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
        if(hStdout != INVALID_HANDLE_VALUE)
        {
            GetConsoleMode(hStdout, &originalMode) && SetConsoleMode(hStdout,
                originalMode|
                ENABLE_VIRTUAL_TERMINAL_PROCESSING|
                DISABLE_NEWLINE_AUTO_RETURN);
        }
    }

    return originalMode;
#else
    return 0;
#endif
}

void restoreConsoleMode(DWORD mode)
{
#ifdef Q_OS_WIN
    if(!isRunningInConsole())
        return;

    auto hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    if(hStdout != INVALID_HANDLE_VALUE)
        SetConsoleMode(hStdout, mode);

    FreeConsole();
#else
    Q_UNUSED(mode);
#endif
}

size_t consoleWidth()
{
    if(isRunningInConsole())
    {
#ifdef Q_OS_WIN
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if(GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
            return csbi.dwSize.X;
#else
        struct winsize w;

        if(ioctl(fileno(stdout), TIOCGWINSZ, &w) == 0)
            return w.ws_col;
#endif
    }

    return 80; // Reasonable default?
}
