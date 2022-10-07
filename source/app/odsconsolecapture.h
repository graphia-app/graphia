/* Copyright © 2013-2022 Graphia Technologies Ltd.
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

#ifndef ODSCONSOLECAPTURE_H
#define ODSCONSOLECAPTURE_H

// ODS - OutputDebugString; windows specific console output

#include <QtGlobal>
#ifdef Q_OS_WIN

#include "consolecapture.h"

#include <atomic>

#include <windows.h>

class ODSCapture : public IConsoleCapture
{
private:
    struct ODSBuffer
    {
        DWORD _processID;
        char  _data[4096 - sizeof(DWORD)];
    };

    ODSBuffer* _buffer = nullptr;
    HANDLE _bufferReadyEvent = nullptr;
    HANDLE _dataReadyEvent = nullptr;

    HANDLE _thread = nullptr;
    std::atomic_bool _stop = false;

    static DWORD WINAPI odsThread(LPVOID arg);

    void stopThread();

public:
    explicit ODSCapture(const QString& filename);
    ~ODSCapture() override;
    void close() override;
};

#endif // Q_OS_WIN

#endif // ODSCONSOLECAPTURE_H
