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

#include "odsconsolecapture.h"

#include "shared/utils/thread.h"

#include <QFile>

DWORD WINAPI ODSCapture::odsThread(LPVOID arg)
{
    u::setCurrentThreadName(QStringLiteral("ODSCapture"));

    const auto* that = static_cast<ODSCapture*>(arg);

    QFile file(that->filename());
    if(!file.open(QIODeviceBase::WriteOnly))
        return -1;

    while(!that->_stop)
    {
        SetEvent(that->_bufferReadyEvent);

        auto result = WaitForSingleObject(that->_dataReadyEvent, INFINITE);
        if(result != WAIT_OBJECT_0 || that->_stop)
            continue;

        auto length = ::strnlen_s(that->_buffer->_data, sizeof(that->_buffer->_data));
        file.write(that->_buffer->_data, length);
        file.flush();
    }

    file.close();

    return 0;
}

ODSCapture::ODSCapture(const QString &filename) :
    IConsoleCapture(filename)
{
    HANDLE mappedFile = nullptr;

    try
    {
        mappedFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(ODSBuffer), L"DBWIN_BUFFER");
        if(mappedFile == nullptr)
            throw 0;

        _buffer = static_cast<ODSBuffer*>(MapViewOfFile(mappedFile, SECTION_MAP_READ, 0, 0, 0));
        if(_buffer == nullptr)
            throw 0;

        _bufferReadyEvent = CreateEvent(NULL, FALSE, FALSE, L"DBWIN_BUFFER_READY");
        if(_bufferReadyEvent == nullptr)
            throw 0;

        _dataReadyEvent = CreateEvent(NULL, FALSE, FALSE, L"DBWIN_DATA_READY");
        if(_dataReadyEvent == nullptr)
            throw 0;

        _thread = CreateThread(NULL, 0, odsThread, this, 0, NULL);
        if(_thread == nullptr)
            throw 0;
    }
    catch(int)
    {
        if(_dataReadyEvent != nullptr)      CloseHandle(_dataReadyEvent);
        if(_bufferReadyEvent != nullptr)    CloseHandle(_bufferReadyEvent);
        if(_buffer != nullptr)              UnmapViewOfFile(_buffer);
        if(mappedFile != nullptr)           CloseHandle(mappedFile);

        _buffer = nullptr;
        _bufferReadyEvent = nullptr;
        _dataReadyEvent = nullptr;
        _thread = nullptr;
    }
}

void ODSCapture::stopThread()
{
    if(_thread != nullptr)
    {
        _stop = true;
        SetEvent(_dataReadyEvent);
        WaitForSingleObject(_thread, INFINITE);
        _thread = nullptr;
    }
}

ODSCapture::~ODSCapture()
{
    stopThread();
}

void ODSCapture::close()
{
    stopThread();
}
