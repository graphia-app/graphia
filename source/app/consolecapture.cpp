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

#include "consolecapture.h"

IConsoleCapture::IConsoleCapture(const QString &filename) :
    _filename(filename) {}

QString IConsoleCapture::filename() const
{
    return _filename;
}

IoStreamCapture::IoStreamCapture(const QString &filename, std::ostream &stream) :
    IConsoleCapture(filename)
{
    _file.open(this->filename().toLocal8Bit().constData());
    stream.rdbuf(_file.rdbuf());
}

IoStreamCapture::~IoStreamCapture()
{
    _file.close();
}

void IoStreamCapture::close()
{
    _file.close();
}

CStreamCapture::CStreamCapture(const QString &filename, FILE *stream) :
    IConsoleCapture(filename)
{
    _file = std::freopen(this->filename().toLocal8Bit().constData(), "w", stream);
}

void CStreamCapture::closeFile()
{
    if(_file != nullptr)
        static_cast<void>(std::fclose(_file));
}

CStreamCapture::~CStreamCapture()
{
    closeFile();
}

void CStreamCapture::close()
{
    closeFile();
}
