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

#ifndef QMLUTILS_H
#define QMLUTILS_H

#include "shared/utils/thread.h"
#include "shared/utils/utils.h"
#include "shared/utils/string.h"
#include "shared/utils/color.h"
#include "shared/utils/crypto.h"
#include "shared/utils/redirects.h"
#include "shared/utils/preferences.h"
#include "shared/utils/apppathname.h"
#include "shared/utils/showinfolder.h"
#include "shared/utils/static_block.h"

#include <QObject>
#include <QUrl>
#include <QDebug>
#include <QFileInfo>
#include <QCollator>
#include <QByteArray>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QCryptographicHash>
#include <QQmlEngine>
#include <QAbstractListModel>

class QQmlEngine;
class QJSEngine;

// NOLINTBEGIN(readability-convert-member-functions-to-static)

class QmlUtils : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(QmlUtils)

public:
    QmlUtils() = default;

    Q_INVOKABLE QString baseFileNameForUrl(const QUrl& url) const { return url.fileName(); }

    Q_INVOKABLE QString extensionForUrl(const QUrl& url) const
    {
        auto fi = QFileInfo(url.toLocalFile());
        return fi.completeSuffix();
    }

    Q_INVOKABLE QString baseFileNameForUrlNoExtension(const QUrl& url) const
    {
        auto fi = QFileInfo(url.toLocalFile());
        return fi.completeBaseName();
    }

    Q_INVOKABLE bool urlIsFile(const QUrl& url) const { return url.isLocalFile(); }

    Q_INVOKABLE bool urlIsDownloadable(const QUrl& url) const
    {
        auto validSchemes = {"http", "https", "ftp"};

        return url.isValid() && std::any_of(std::begin(validSchemes), std::end(validSchemes),
        [&url](const auto& scheme)
        {
            return url.scheme() == scheme;
        });
    }

    Q_INVOKABLE QString fileNameForUrl(const QUrl& url) const { return url.toLocalFile(); }
    Q_INVOKABLE QUrl urlForFileName(const QString& fileName) const { return QUrl::fromLocalFile(fileName); }
    Q_INVOKABLE QUrl urlForUserInput(const QString& userInput) const { return QUrl::fromUserInput(userInput); }
    Q_INVOKABLE bool fileExists(const QString& fileName) const { return QFileInfo::exists(fileName); }
    Q_INVOKABLE bool fileUrlExists(const QUrl& url) const { return QFileInfo::exists(url.toLocalFile()); }

    Q_INVOKABLE QUrl replaceExtension(const QUrl& url, const QString& extension) const
    {
        auto fi = QFileInfo(url.toLocalFile());
        auto replaced = QFileInfo(QStringLiteral("%1/%2.%3")
            .arg(fi.path(), fi.completeBaseName(), extension));

        return QUrl::fromLocalFile(replaced.filePath());
    }

    Q_INVOKABLE QString currentThreadName() const { return u::currentThreadName(); }

    Q_INVOKABLE bool urlIsValid(const QUrl& url) const
    {
        return url.isValid();
    }

    Q_INVOKABLE bool urlStringIsValid(const QString& urlString) const
    {
        QUrl url = QUrl(urlString, QUrl::ParsingMode::StrictMode);
        auto validSchemes = {"http", "https", "ftp", "file"};

        return url.isValid() && std::any_of(std::begin(validSchemes), std::end(validSchemes),
        [&url](const auto& scheme)
        {
            return url.scheme() == scheme;
        });
    }

    Q_INVOKABLE bool userUrlStringIsValid(const QString& urlString) const
    {
        QUrl url = QUrl::fromUserInput(urlString);
        return url.isValid();
    }

    Q_INVOKABLE QString urlFrom(const QString& userUrlString) const
    {
        QUrl url = QUrl::fromUserInput(userUrlString);
        return url.toString();
    }

    // QML JS comparelocale doesn't include numeric implementation...
    Q_INVOKABLE int localeCompareStrings(const QString& left, const QString& right, bool numeric = true)
    {
        _collator.setNumericMode(numeric);
        return _collator.compare(left, right);
    }

    static QObject* qmlInstance(QQmlEngine*, QJSEngine*)
    {
        return new QmlUtils;
    }

    Q_INVOKABLE QString formatNumberScientific(double value) const
    {
        return u::formatNumberScientific(value);
    }

    Q_INVOKABLE QString formatNumberSIPostfix(double value) const
    {
        return u::formatNumberSIPostfix(value);
    }

    Q_INVOKABLE QColor contrastingColor(const QColor& color) const
    {
        return u::contrastingColor(color);
    }

    Q_INVOKABLE QColor colorForString(const QString& string) const
    {
        return u::colorForString(string);
    }

    Q_INVOKABLE QString base64EncodingOf(const QString& filename) const
    {
        QFile file(filename);

        if(!file.open(QIODevice::ReadOnly))
            return {};

        return file.readAll().toBase64();
    }

    Q_INVOKABLE QByteArray byteArrayFromBase64String(const QString& base64String) const
    {
        return QByteArray::fromBase64(base64String.toLatin1());
    }

    Q_INVOKABLE QString tempDirectory() const
    {
        QTemporaryDir tempDir;
        tempDir.setAutoRemove(false);

        if(!tempDir.isValid())
            return {};

        return tempDir.path();
    }

    Q_INVOKABLE bool cd(const QString& dirName) const
    {
        return QDir::setCurrent(dirName);
    }

    Q_INVOKABLE bool rmdir(const QString& dirName) const
    {
        QDir dir(dirName);
        return dir.removeRecursively();
    }

    Q_INVOKABLE bool copy(const QString& from, const QString& to) const
    {
        return QFile::copy(from, to);
    }

    Q_INVOKABLE QString sha256(const QByteArray& data) const
    {
        return QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex();
    }

    Q_INVOKABLE QByteArray filterHtmlHack(const QByteArray& data) const
    {
        if(!data.startsWith("<html>"))
            return data;

        auto index = data.indexOf("</html>") + 7;
        return data.mid(index);
    }

    Q_INVOKABLE QByteArray readFromFile(const QString& filename) const
    {
        QFile file(filename);

        if(!file.open(QIODevice::ReadOnly))
            return {};

        return file.readAll();
    }

    Q_INVOKABLE bool writeToFile(const QString& filename, const QByteArray& data) const
    {
        QFile file(filename);

        if(!file.open(QIODevice::ReadWrite | QIODevice::Truncate))
            return {};

        return file.write(data) == data.size();
    }

    Q_INVOKABLE QString bytesToHexString(const QByteArray& data) const
    {
        return data.toHex();
    }

    Q_INVOKABLE QString stringAsHexString(const QString& data) const
    {
        return bytesToHexString(data.toUtf8());
    }

    Q_INVOKABLE QByteArray hexStringAsBytes(const QString& data) const
    {
        return QByteArray::fromHex(data.toUtf8());
    }

    Q_INVOKABLE QString hexStringAsString(const QString& data) const
    {
        return hexStringAsBytes(data.toUtf8());
    }

    Q_INVOKABLE bool isHexString(const QString& string) const
    {
        return u::isHex(string);
    }

    Q_INVOKABLE QString rsaSignatureForString(const QString& string, const QString& keyFilename) const
    {
        auto signature = u::rsaSignString(string.toStdString(),
            fileNameForUrl(keyFilename).toStdString());

        return QString::fromStdString(u::bytesToHex(signature));
    }

    Q_INVOKABLE QString redirectUrl(const QString& shortName) const
    {
        return QStringLiteral("%1/%2").arg(u::getPref(QStringLiteral("servers/redirects")).toString(), shortName);
    }

    Q_INVOKABLE QString redirectLink(const QString& shortName, const QString& linkText) const
    {
        return u::redirectLink(shortName.toLocal8Bit().data(), linkText);
    }

    Q_INVOKABLE void showAppInFileManager() const
    {
        u::showInFolder(u::appPathName());
    }

    Q_INVOKABLE int modelRoleForName(QAbstractItemModel* model, const QByteArray& roleName) const
    {
        Q_ASSERT(model != nullptr);
        if(model == nullptr)
            return -1;

        return model->roleNames().key(roleName);
    }

private:
    QCollator _collator;
};

// NOLINTEND(readability-convert-member-functions-to-static)

static_block
{
    qmlRegisterSingletonType<QmlUtils>(
        APP_URI, APP_MAJOR_VERSION, APP_MINOR_VERSION, "QmlUtils", &QmlUtils::qmlInstance);
}

#endif // QMLUTILS_H
