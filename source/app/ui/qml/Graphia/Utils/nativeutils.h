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

#ifndef NATIVEUTILS_H
#define NATIVEUTILS_H

#include <QObject>
#include <QQmlEngine>
#include <QString>
#include <QUrl>
#include <QColor>
#include <QByteArray>

class QAbstractItemModel;

using namespace Qt::Literals::StringLiterals;

class NativeUtils : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QString validAttributeNameRegex READ validAttributeNameRegex CONSTANT)

public:
    Q_INVOKABLE QString baseFileNameForUrl(const QUrl& url) const;
    Q_INVOKABLE QString extensionForUrl(const QUrl& url) const;
    Q_INVOKABLE QString baseFileNameForUrlNoExtension(const QUrl& url) const;

    Q_INVOKABLE bool urlIsFile(const QUrl& url) const;
    Q_INVOKABLE bool urlIsDownloadable(const QUrl& url) const;

    Q_INVOKABLE QString fileNameForUrl(const QUrl& url) const;
    Q_INVOKABLE QUrl urlForFileName(const QString& fileName) const;
    Q_INVOKABLE QUrl urlForUserInput(const QString& userInput) const;
    Q_INVOKABLE bool fileExists(const QString& fileName) const;
    Q_INVOKABLE bool fileUrlExists(const QUrl& url) const;

    Q_INVOKABLE QUrl removeExtension(const QUrl& url) const;

    Q_INVOKABLE QString currentThreadName() const;

    Q_INVOKABLE bool urlIsValid(const QUrl& url) const;
    Q_INVOKABLE bool urlStringIsValid(const QString& urlString) const;
    Q_INVOKABLE bool userUrlStringIsValid(const QString& urlString) const;
    Q_INVOKABLE QString urlFrom(const QString& userUrlString) const;

    Q_INVOKABLE int compareStrings(const QString& left, const QString& right, bool numeric = true);

    Q_INVOKABLE QString formatNumber(double value) const;
    Q_INVOKABLE QString formatNumberScientific(double value) const;
    Q_INVOKABLE QString formatNumberSIPostfix(double value) const;

    Q_INVOKABLE QColor contrastingColor(const QColor& color) const;
    Q_INVOKABLE QColor colorForString(const QString& string) const;

    Q_INVOKABLE QString htmlEscape(const QString& string) const;

    Q_INVOKABLE QString base64EncodingOf(const QString& filename) const;
    Q_INVOKABLE QByteArray byteArrayFromBase64String(const QString& base64String) const;

    Q_INVOKABLE QString tempDirectory() const;
    Q_INVOKABLE bool cd(const QString& dirName) const;
    Q_INVOKABLE bool rmdir(const QString& dirName) const;
    Q_INVOKABLE bool copy(const QString& from, const QString& to) const;

    Q_INVOKABLE QString sha256(const QByteArray& data) const;

    Q_INVOKABLE QByteArray filterHtmlHack(const QByteArray& data) const;

    Q_INVOKABLE QByteArray readFromFile(const QString& filename) const;
    Q_INVOKABLE bool writeToFile(const QString& filename, const QByteArray& data) const;

    Q_INVOKABLE QString bytesToHexString(const QByteArray& data) const;
    Q_INVOKABLE QString stringAsHexString(const QString& data) const;
    Q_INVOKABLE QByteArray hexStringAsBytes(const QString& data) const;
    Q_INVOKABLE QString hexStringAsString(const QString& data) const;
    Q_INVOKABLE bool isHexString(const QString& string) const;

    Q_INVOKABLE QString rsaSignatureForString(const QString& string, const QString& keyFilename) const;

    Q_INVOKABLE QString redirectUrl(const QString& shortName) const;
    Q_INVOKABLE QString redirectLink(const QString& shortName, const QString& linkText) const;

    Q_INVOKABLE void showAppInFileManager() const;

    Q_INVOKABLE int modelRoleForName(QAbstractItemModel* model, const QByteArray& roleName) const;

    QString validAttributeNameRegex() const;

    Q_INVOKABLE QString sanitiseAttributeName(const QString& attributeName) const;

    Q_INVOKABLE QString nativeShortcutSequence(const QString& shortcut);
};

#endif // NATIVEUTILS_H
