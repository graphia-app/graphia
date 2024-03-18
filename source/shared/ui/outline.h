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

#ifndef OUTLINE_H
#define OUTLINE_H

#include "shared/utils/static_block.h"
#include "shared/utils/qmlcontrolcolors.h"

#include <QQmlEngine>
#include <QQuickPaintedItem>

class Outline : public QQuickPaintedItem
{
    Q_OBJECT

    Q_PROPERTY(bool outlineVisible MEMBER _outlineVisible NOTIFY outlineVisibleChanged)
    Q_PROPERTY(double outlineWidth MEMBER _outlineWidth NOTIFY outlineWidthChanged)

public:
    explicit Outline(QQuickItem* parent = nullptr);

    void paint(QPainter *painter) override;

private:
    QmlControlColors _qmlControlColors;

    bool _outlineVisible = true;
    double _outlineWidth = 1.0;
    double _widthModifier = 1.0;
    QColor _color = Qt::black;

signals:
    void outlineVisibleChanged();
    void outlineWidthChanged();
};

static_block
{
    qmlRegisterType<Outline>(
        APP_URI, APP_MAJOR_VERSION, APP_MINOR_VERSION, "Outline");
}

#endif // OUTLINE_H
