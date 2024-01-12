/* Copyright © 2013-2023 Graphia Technologies Ltd.
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

#ifndef ICONITEM_H
#define ICONITEM_H

#include "shared/utils/static_block.h"

#include <QQmlEngine>
#include <QQuickPaintedItem>
#include <QIcon>
#include <QString>

class IconItem : public QQuickPaintedItem
{
    Q_OBJECT

    Q_PROPERTY(QString iconName READ iconName WRITE setIconName NOTIFY iconNameChanged)
    Q_PROPERTY(bool on MEMBER _on NOTIFY onChanged)
    Q_PROPERTY(bool selected MEMBER _selected NOTIFY selectedChanged)
    Q_PROPERTY(bool valid READ valid NOTIFY validChanged)

public:
    explicit IconItem(QQuickItem* parent = nullptr);

    void paint(QPainter *painter) override;

    QString iconName() const { return _iconName; }
    void setIconName(const QString& iconName);
    bool valid() const { return !_icon.isNull(); }

private:
    QString _iconName;
    bool _on = false;
    bool _selected = false;
    QIcon _icon;

signals:
    void iconNameChanged();
    void onChanged();
    void selectedChanged();
    void validChanged();
};

static_block
{
    qmlRegisterType<IconItem>(
        APP_URI, APP_MAJOR_VERSION, APP_MINOR_VERSION, "NamedIcon");
}

#endif // ICONITEM_H
