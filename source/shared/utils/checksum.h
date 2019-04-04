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
