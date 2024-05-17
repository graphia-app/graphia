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

#include "nativeutils.h"

#include "shared/utils/thread.h"
#include "shared/utils/utils.h"
#include "shared/utils/string.h"
#include "shared/utils/color.h"
#include "shared/utils/crypto.h"
#include "shared/utils/redirects.h"
#include "shared/utils/preferences.h"
#include "shared/utils/apppathname.h"
#include "shared/utils/showinfolder.h"
#include "shared/attributes/iattribute.h"

#include <QObject>
#include <QUrl>
#include <QDebug>
#include <QFileInfo>
#include <QByteArray>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QCryptographicHash>
#include <QAbstractListModel>
#include <QKeySequence>

QString NativeUtils::baseFileNameForUrl(const QUrl& url) const
{
    return url.fileName();
}

QString NativeUtils::extensionForUrl(const QUrl& url) const
{
    auto fi = QFileInfo(url.toLocalFile());
    return fi.suffix();
}

QString NativeUtils::baseFileNameForUrlNoExtension(const QUrl& url) const
{
    auto fi = QFileInfo(url.toLocalFile());
    return fi.completeBaseName();
}

bool NativeUtils::urlIsFile(const QUrl& url) const { return url.isLocalFile(); }

bool NativeUtils::urlIsDownloadable(const QUrl& url) const
{
    auto validSchemes = {"http", "https", "ftp"};

    return url.isValid() && std::any_of(std::begin(validSchemes), std::end(validSchemes),
    [&url](const auto& scheme)
    {
        return url.scheme() == scheme;
    });
}

QString NativeUtils::fileNameForUrl(const QUrl& url) const
{
    return url.toLocalFile();
}

QUrl NativeUtils::urlForFileName(const QString& fileName) const
{
    return QUrl::fromLocalFile(fileName);
}

QUrl NativeUtils::urlForUserInput(const QString& userInput) const
{
    return QUrl::fromUserInput(userInput);
}

bool NativeUtils::fileExists(const QString& fileName) const
{
    return QFileInfo::exists(fileName);
}

bool NativeUtils::fileUrlExists(const QUrl& url) const
{
    return QFileInfo::exists(url.toLocalFile());
}

QUrl NativeUtils::removeExtension(const QUrl& url) const
{
    auto fi = QFileInfo(url.toLocalFile());
    auto removed = QFileInfo(u"%1/%2"_s.arg(fi.path(), fi.completeBaseName()));

    return QUrl::fromLocalFile(removed.filePath());
}

QString NativeUtils::currentThreadName() const
{
    return u::currentThreadName();
}

bool NativeUtils::urlIsValid(const QUrl& url) const
{
    return url.isValid();
}

bool NativeUtils::urlStringIsValid(const QString& urlString) const
{
    QUrl url = QUrl(urlString, QUrl::ParsingMode::StrictMode);
    auto validSchemes = {"http", "https", "ftp", "file"};

    return url.isValid() && std::any_of(std::begin(validSchemes), std::end(validSchemes),
    [&url](const auto& scheme)
    {
        return url.scheme() == scheme;
    });
}

bool NativeUtils::userUrlStringIsValid(const QString& urlString) const
{
    const QUrl url = QUrl::fromUserInput(urlString);
    return url.isValid();
}

QString NativeUtils::urlFrom(const QString& userUrlString) const
{
    const QUrl url = QUrl::fromUserInput(userUrlString);
    return url.toString();
}

// QML JS comparelocale doesn't include numeric implementation...
int NativeUtils::compareStrings(const QString& left, const QString& right, bool numeric)
{
    return numeric ? u::numericCompare(left, right) : QString::compare(left, right);
}

QString NativeUtils::formatNumber(double value) const
{
    return u::formatNumber(value);
}

QString NativeUtils::formatNumberScientific(double value) const
{
    return u::formatNumberScientific(value);
}

QString NativeUtils::formatNumberSIPostfix(double value) const
{
    return u::formatNumberSIPostfix(value);
}

QColor NativeUtils::contrastingColor(const QColor& color) const
{
    return u::contrastingColor(color);
}

QColor NativeUtils::colorForString(const QString& string) const
{
    return u::colorForString(string);
}

QString NativeUtils::htmlEscape(const QString& string) const
{
    return string.toHtmlEscaped();
}

QString NativeUtils::base64EncodingOf(const QString& filename) const
{
    QFile file(filename);

    if(!file.open(QIODevice::ReadOnly))
        return {};

    return file.readAll().toBase64();
}

QByteArray NativeUtils::byteArrayFromBase64String(const QString& base64String) const
{
    return QByteArray::fromBase64(base64String.toLatin1());
}

QString NativeUtils::tempDirectory() const
{
    QTemporaryDir tempDir;
    tempDir.setAutoRemove(false);

    if(!tempDir.isValid())
        return {};

    return tempDir.path();
}

bool NativeUtils::cd(const QString& dirName) const
{
    return QDir::setCurrent(dirName);
}

bool NativeUtils::rmdir(const QString& dirName) const
{
    QDir dir(dirName);
    return dir.removeRecursively();
}

bool NativeUtils::copy(const QString& from, const QString& to) const
{
    return QFile::copy(from, to);
}

QString NativeUtils::sha256(const QByteArray& data) const
{
    return QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex();
}

QByteArray NativeUtils::filterHtmlHack(const QByteArray& data) const
{
    if(!data.startsWith("<html>"))
        return data;

    auto index = data.indexOf("</html>") + 7;
    return data.mid(index);
}

QByteArray NativeUtils::readFromFile(const QString& filename) const
{
    QFile file(filename);

    if(!file.open(QIODevice::ReadOnly))
        return {};

    return file.readAll();
}

bool NativeUtils::writeToFile(const QString& filename, const QByteArray& data) const
{
    QFile file(filename);

    if(!file.open(QIODevice::ReadWrite | QIODevice::Truncate))
        return {};

    return file.write(data) == data.size();
}

QString NativeUtils::bytesToHexString(const QByteArray& data) const
{
    return data.toHex();
}

QString NativeUtils::stringAsHexString(const QString& data) const
{
    return bytesToHexString(data.toUtf8());
}

QByteArray NativeUtils::hexStringAsBytes(const QString& data) const
{
    return QByteArray::fromHex(data.toUtf8());
}

QString NativeUtils::hexStringAsString(const QString& data) const
{
    return hexStringAsBytes(data.toUtf8());
}

bool NativeUtils::isHexString(const QString& string) const
{
    return u::isHex(string);
}

QString NativeUtils::rsaSignatureForString(const QString& string, const QString& keyFilename) const
{
    auto signature = u::rsaSignString(string.toStdString(),
                                      fileNameForUrl(keyFilename).toStdString());

    return QString::fromStdString(u::bytesToHex(signature));
}

QString NativeUtils::redirectUrl(const QString& shortName) const
{
    return u"%1/%2"_s.arg(u::getPref(u"servers/redirects"_s).toString(), shortName);
}

QString NativeUtils::redirectLink(const QString& shortName, const QString& linkText) const
{
    return u::redirectLink(shortName.toLocal8Bit().data(), linkText);
}

void NativeUtils::showAppInFileManager() const
{
    u::showInFolder(u::appPathName());
}

int NativeUtils::modelRoleForName(QAbstractItemModel* model, const QByteArray& roleName) const
{
    Q_ASSERT(model != nullptr);
    if(model == nullptr)
        return -1;

    return model->roleNames().key(roleName);
}

QString NativeUtils::validAttributeNameRegex() const
{
    return IAttribute::ValidNameRegex;
}

QString NativeUtils::sanitiseAttributeName(const QString& attributeName) const
{
    // Strip off the ^ and $
    QString partialRegex(IAttribute::ValidNameRegex);
    partialRegex = partialRegex.mid(1, partialRegex.length() - 2);
    const QRegularExpression re(partialRegex);

    auto match = re.match(attributeName, 0, QRegularExpression::PartialPreferCompleteMatch);
    if(match.hasMatch())
        return match.captured();

    return {};
}

QString NativeUtils::nativeShortcutSequence(const QString& shortcut)
{
    return QKeySequence(shortcut, QKeySequence::PortableText)
        .toString(QKeySequence::NativeText);
}
