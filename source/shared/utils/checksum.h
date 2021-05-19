/* Copyright © 2013-2021 Graphia Technologies Ltd.
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

#ifndef CHECKSUM_H
#define CHECKSUM_H

#include <QString>
#include <QFile>
#include <QCryptographicHash>

namespace u
{
    static inline bool sha256ChecksumMatchesFile(const QString& fileName, const QString& checksum)
    {
        QFile file(fileName);
        if(!file.open(QFile::ReadOnly))
            return false;

        QCryptographicHash sha256(QCryptographicHash::Sha256);

        if(!sha256.addData(&file))
            return false;

        auto checksumOfFile = QString(sha256.result().toHex());

        return checksum == checksumOfFile;
    }
} // namespace u

#endif // CHECKSUM_H
