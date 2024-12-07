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

#ifndef NAMEDICON_H
#define NAMEDICON_H

#include <QQuickPaintedItem>
#include <QIcon>
#include <QString>

class NamedIcon : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString iconName READ iconName WRITE setIconName NOTIFY iconNameChanged)
    Q_PROPERTY(bool on MEMBER _on NOTIFY onChanged)
    Q_PROPERTY(bool selected MEMBER _selected NOTIFY selectedChanged)
    Q_PROPERTY(bool fill MEMBER _fill NOTIFY fillChanged)
    Q_PROPERTY(bool valid READ valid NOTIFY validChanged)

public:
    explicit NamedIcon(QQuickItem* parent = nullptr);

    void paint(QPainter *painter) override;

    QString iconName() const { return _iconName; }
    void setIconName(const QString& iconName);
    bool valid() const { return !_icon.isNull(); }

private:
    QString _iconName;
    bool _on = false;
    bool _selected = false;
    bool _fill = false;
    QIcon _icon;

signals:
    void iconNameChanged();
    void onChanged();
    void selectedChanged();
    void fillChanged();
    void validChanged();
};

#endif // NAMEDICON_H
