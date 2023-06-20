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

#include "consolecapture.h"
#include "odsconsolecapture.h"
#include "debugger.h"
#include "console.h"

#include <iostream>

using namespace Qt::Literals::StringLiterals;

IConsoleCapture::IConsoleCapture(const QString &filename) :
    _filename(filename) {}

QString IConsoleCapture::filename() const
{
    return _filename;
}

IoStreamCapture::IoStreamCapture(const QString &filename, std::ostream &stream) :
    IConsoleCapture(filename), _stream(&stream)
{
    _file.open(this->filename().toLocal8Bit().constData());

    if(_file.is_open())
        _originalStreambuf = stream.rdbuf(_file.rdbuf());
}

void IoStreamCapture::closeFile()
{
    if(_file.is_open())
    {
        // Restore prior state
        _stream->rdbuf(_originalStreambuf);
        _originalStreambuf = nullptr;
        _stream = nullptr;

        _file.close();
    }
}

IoStreamCapture::~IoStreamCapture()
{
    closeFile();
}

void IoStreamCapture::close()
{
    closeFile();
}

CStreamCapture::CStreamCapture(const QString &filename, FILE *stream) :
    IConsoleCapture(filename)
{
    _file = std::freopen(this->filename().toLocal8Bit().constData(), "w", stream);
}

void CStreamCapture::closeFile()
{
    if(_file != nullptr)
    {
        static_cast<void>(std::fclose(_file));
        _file = nullptr;
    }
}

CStreamCapture::~CStreamCapture()
{
    closeFile();
}

void CStreamCapture::close()
{
    closeFile();
}

ConsoleOutputFiles captureConsoleOutput(const QString& path, const QString& prefix)
{
    if(isRunningInConsole() || u::isDebuggerPresent())
        return {};

    auto filename = [&](const char* basename)
    {
        if(!prefix.isEmpty())
            return u"%1/%2_%3.txt"_s.arg(path, prefix, basename);

        return u"%1/%2.txt"_s.arg(path, basename);
    };

    return
    {
        std::make_shared<IoStreamCapture>(filename("cout"), std::cout),
        std::make_shared<IoStreamCapture>(filename("cerr"), std::cerr),
        std::make_shared<CStreamCapture>(filename("stdout"), stdout),
        std::make_shared<CStreamCapture>(filename("stderr"), stderr),
#ifdef Q_OS_WIN
        std::make_shared<ODSCapture>(filename("outputdebugstring")),
#endif
    };
}
