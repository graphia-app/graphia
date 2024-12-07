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

#ifndef CONSOLECAPTURE_H
#define CONSOLECAPTURE_H

#include <QString>

#include <fstream>
#include <cstdio>

class IConsoleCapture
{
private:
    QString _filename;

public:
    explicit IConsoleCapture(const QString& filename);
    virtual ~IConsoleCapture() = default;

    virtual void close() {}
    QString filename() const;
};

class IoStreamCapture : public IConsoleCapture
{
private:
    std::ofstream _file;
    std::ostream* _stream = nullptr;
    std::streambuf* _originalStreambuf = nullptr;

    void closeFile();

public:
    IoStreamCapture(const QString& filename, std::ostream& stream);
    ~IoStreamCapture() override;
    void close() override;
};

class CStreamCapture : public IConsoleCapture
{
private:
    std::FILE* _file = nullptr;

    void closeFile();

public:
    CStreamCapture(const QString& filename, std::FILE* stream);
    ~CStreamCapture() override;
    void close() override;
};

using ConsoleOutputFiles = std::vector<std::shared_ptr<IConsoleCapture>>;
ConsoleOutputFiles captureConsoleOutput(const QString& path, const QString& prefix = {});

#endif // CONSOLECAPTURE_H
