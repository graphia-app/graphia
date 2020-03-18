/* Copyright © 2013-2020 Graphia Technologies Ltd.
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

#ifndef QMLUTILS_H
#define QMLUTILS_H

#include "shared/utils/thread.h"
#include "shared/utils/utils.h"
#include "shared/utils/string.h"
#include "shared/utils/color.h"

#include <QObject>
#include <QUrl>
#include <QDebug>
#include <QFileInfo>
#include <QCollator>
#include <QByteArray>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QByteArray>
#include <QCryptographicHash>

class QQmlEngine;
class QJSEngine;

class QmlUtils : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(QmlUtils)

public:
    QmlUtils() = default;

    // NOLINTNEXTLINE readability-convert-member-functions-to-static
    Q_INVOKABLE QString baseFileNameForUrl(const QUrl& url) const { return url.fileName(); }

    // NOLINTNEXTLINE readability-convert-member-functions-to-static
    Q_INVOKABLE QString baseFileNameForUrlNoExtension(const QUrl& url) const
    {
        auto fi = QFileInfo(url.toLocalFile());
        return fi.baseName();
    }

    // NOLINTNEXTLINE readability-convert-member-functions-to-static
    Q_INVOKABLE QString fileNameForUrl(const QUrl& url) const { return url.toLocalFile(); }
    // NOLINTNEXTLINE readability-convert-member-functions-to-static
    Q_INVOKABLE QUrl urlForFileName(const QString& fileName) const { return QUrl::fromLocalFile(fileName); }
    // NOLINTNEXTLINE readability-convert-member-functions-to-static
    Q_INVOKABLE QUrl urlForUserInput(const QString& userInput) const { return QUrl::fromUserInput(userInput); }
    // NOLINTNEXTLINE readability-convert-member-functions-to-static
    Q_INVOKABLE bool fileExists(const QString& fileName) const { return QFileInfo::exists(fileName); }
    // NOLINTNEXTLINE readability-convert-member-functions-to-static
    Q_INVOKABLE bool fileUrlExists(const QUrl& url) const { return QFileInfo::exists(url.toLocalFile()); }

    // NOLINTNEXTLINE readability-convert-member-functions-to-static
    Q_INVOKABLE QUrl replaceExtension(const QUrl& url, const QString& extension) const
    {
        auto fi = QFileInfo(url.toLocalFile());
        auto replaced = QFileInfo(QStringLiteral("%1/%2.%3")
                                  .arg(fi.path(),
                                       fi.baseName(),
                                       extension));

        return QUrl::fromLocalFile(replaced.filePath());
    }

    // NOLINTNEXTLINE readability-convert-member-functions-to-static
    Q_INVOKABLE QString currentThreadName() const { return u::currentThreadName(); }

    // NOLINTNEXTLINE readability-convert-member-functions-to-static
    Q_INVOKABLE bool urlIsValid(const QString& urlString) const
    {
        QUrl url = QUrl::fromUserInput(urlString);
        return url.isValid();
    }

    // QML JS comparelocale doesn't include numeric implementation...
    // NOLINTNEXTLINE readability-convert-member-functions-to-static
    Q_INVOKABLE int localeCompareStrings(const QString& left, const QString& right, bool numeric = true)
    {
        _collator.setNumericMode(numeric);
        return _collator.compare(left, right);
    }

    static QObject* qmlInstance(QQmlEngine*, QJSEngine*)
    {
        return new QmlUtils;
    }

    // NOLINTNEXTLINE readability-convert-member-functions-to-static
    Q_INVOKABLE QString formatNumberScientific(double value)
    {
        return u::formatNumberScientific(value);
    }

    // NOLINTNEXTLINE readability-convert-member-functions-to-static
    Q_INVOKABLE QString formatNumberSIPostfix(double value)
    {
        return u::formatNumberSIPostfix(value);
    }

    // NOLINTNEXTLINE readability-convert-member-functions-to-static
    Q_INVOKABLE QColor contrastingColor(const QColor& color)
    {
        return u::contrastingColor(color);
    }

    // NOLINTNEXTLINE readability-convert-member-functions-to-static
    Q_INVOKABLE QColor colorForString(const QString& string)
    {
        return u::colorForString(string);
    }

    // NOLINTNEXTLINE readability-convert-member-functions-to-static
    Q_INVOKABLE QString base64EncodingOf(const QString& filename)
    {
        QFile file(filename);

        if(!file.open(QIODevice::ReadOnly))
            return {};

        return file.readAll().toBase64();
    }

    // NOLINTNEXTLINE readability-convert-member-functions-to-static
    Q_INVOKABLE QByteArray byteArrayFromBase64String(const QString& base64String)
    {
        return QByteArray::fromBase64(base64String.toLatin1());
    }

    // NOLINTNEXTLINE readability-convert-member-functions-to-static
    Q_INVOKABLE QString tempDirectory()
    {
        QTemporaryDir tempDir;
        tempDir.setAutoRemove(false);

        if(!tempDir.isValid())
            return {};

        return tempDir.path();
    }

    // NOLINTNEXTLINE readability-convert-member-functions-to-static
    Q_INVOKABLE bool cd(const QString& dirName)
    {
        return QDir::setCurrent(dirName);
    }

    // NOLINTNEXTLINE readability-convert-member-functions-to-static
    Q_INVOKABLE bool rmdir(const QString& dirName)
    {
        QDir dir(dirName);
        return dir.removeRecursively();
    }

    // NOLINTNEXTLINE readability-convert-member-functions-to-static
    Q_INVOKABLE bool copy(const QString& from, const QString& to)
    {
        return QFile::copy(from, to);
    }

    // NOLINTNEXTLINE readability-convert-member-functions-to-static
    Q_INVOKABLE QString sha256(const QByteArray& data)
    {
        return QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex();
    }

    // NOLINTNEXTLINE readability-convert-member-functions-to-static
    Q_INVOKABLE QByteArray readFromFile(const QString& filename)
    {
        QFile file(filename);

        if(!file.open(QIODevice::ReadOnly))
            return {};

        return file.readAll();
    }

    // NOLINTNEXTLINE readability-convert-member-functions-to-static
    Q_INVOKABLE bool writeToFile(const QString& filename, const QByteArray& data)
    {
        QFile file(filename);

        if(!file.open(QIODevice::ReadWrite | QIODevice::Truncate))
            return {};

        return file.write(data) == data.size();
    }

private:
    QCollator _collator;
};

#endif // QMLUTILS_H
