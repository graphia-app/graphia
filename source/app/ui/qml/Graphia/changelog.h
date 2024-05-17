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

#ifndef CHANGELOG_H
#define CHANGELOG_H

#include <QObject>
#include <QQmlEngine>
#include <QString>
#include <QTemporaryDir>

class ChangeLog : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString text MEMBER _text NOTIFY textChanged())
    Q_PROPERTY(bool available MEMBER _available NOTIFY availableChanged)

public:
    explicit ChangeLog(QObject *parent = nullptr);

    Q_INVOKABLE void refresh();

private:
    QString _text = tr("No changes available.");
    bool _available = false;
    QTemporaryDir _imagesDirectory;

signals:
    void textChanged();
    void availableChanged();
};

#endif // CHANGELOG_H
