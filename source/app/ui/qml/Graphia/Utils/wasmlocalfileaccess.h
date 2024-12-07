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

#ifndef WASMLOCALFILEACCESS_H
#define WASMLOCALFILEACCESS_H

#include <QObject>
#include <QQmlEngine>
#include <QStringList>
#include <QUrl>
#include <QQuickItem>
#include <QQmlEngine>

class WasmLocalFileAccess : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QUrl fileUrl MEMBER _fileUrl NOTIFY fileUrlChanged)

public:
    explicit WasmLocalFileAccess(QQuickItem* parent = nullptr);

    Q_INVOKABLE void open(const QStringList& nameFilters);
    Q_INVOKABLE QUrl save(const QString& filenameHint);

private:
    QUrl _fileUrl;

signals:
    void fileUrlChanged();
    void accepted();
    void rejected();
};

#endif // WASMLOCALFILEACCESS_H
