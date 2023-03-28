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

#include "signalhandling.h"

#include <QCoreApplication>
#include <QtGlobal>

#include <iostream>

#ifdef Q_OS_WIN
#include <Windows.h>
#else
#include <csignal>
#include <unistd.h>
#endif

void installSignalHandlers()
{
#ifdef Q_OS_WIN
    SetConsoleCtrlHandler([](DWORD ctrlType)
    {
        switch(ctrlType)
        {
        case CTRL_C_EVENT:
        case CTRL_CLOSE_EVENT:
            std::cerr << "\nCaught CTRL event " << ctrlType << ", quitting...\n";
            QCoreApplication::quit();
            return TRUE;

        default:
            return FALSE;
        }
    }, TRUE);
#else
    auto handler = [](int sig)
    {
        std::cerr << "\nCaught signal " << sig << ", quitting...\n";
        QCoreApplication::quit();
    };

    auto quitSignals = {SIGQUIT, SIGINT, SIGTERM, SIGHUP};

    sigset_t mask;
    sigemptyset(&mask);
    for(auto signal : quitSignals)
        sigaddset(&mask, signal);

    struct sigaction sa;
    sa.sa_handler = handler;
    sa.sa_mask = mask;
    sa.sa_flags = 0;

    for(auto signal : quitSignals)
        sigaction(signal, &sa, nullptr);
#endif
}
