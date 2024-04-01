/* Copyright © 2013-2024 Graphia Technologies Ltd.
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

#include "qrcextract.h"

#include <QString>
#include <QDir>
#include <QFile>
#include <QDebug>

#include <utility>

using namespace Qt::Literals::StringLiterals;

void u::qrcExtract(const QString& prefix, const QString& destination, const QString& source)
{
    auto fqSource(u"%1%2"_s.arg(prefix, source));
    auto fqDestination = u"%1%2"_s.arg(destination, source);

    auto files = QDir(fqSource).entryList(QDir::Files);
    auto directories = QDir(fqSource).entryList(QDir::Dirs|QDir::NoDotAndDotDot);

    for(const auto& file : std::as_const(files))
    {
        if(!QDir().mkpath(fqDestination))
        {
            qWarning() << "Failed to create path" << fqDestination;
            continue;
        }

        auto fqSourceFile = u"%1/%2"_s.arg(fqSource, file);
        auto fqDestinationFile = u"%1/%2"_s.arg(fqDestination, file);

        if(!QFile::copy(fqSourceFile, fqDestinationFile))
            qWarning() << "Failed to extract" << fqSourceFile << "to" << fqDestinationFile;
    }

    for(const auto& directory : std::as_const(directories))
        qrcExtract(prefix, destination, u"%1/%2"_s.arg(source, directory));
}
