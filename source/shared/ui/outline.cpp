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

#include "outline.h"

#include <QPainter>
#include <QQuickWindow>
#include <QtGlobal>

Outline::Outline(QQuickItem* parent) : QQuickPaintedItem(parent)
{
    connect(this, &Outline::outlineVisibleChanged, [this] { update(); });
    connect(this, &Outline::outlineWidthChanged, [this] { update(); });
    connect(&_qmlControlColors, &QmlControlColors::paletteChanged, [this] { update(); });

#ifdef Q_OS_MACOS
    // On macOS, outlines tend to be a single screen pixel, even on Retina displays
    connect(this, &QQuickPaintedItem::windowChanged, [this]
    {
        if(window() != nullptr)
            _widthModifier = 1.0 / window()->devicePixelRatio();

        update();
    });
#endif
}

void Outline::paint(QPainter* painter)
{
    if(!_outlineVisible)
        return;

    const qreal outlineWidth = _outlineWidth * _widthModifier;
    const qreal halfOutlineWidth = outlineWidth * 0.5;

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QPen(_qmlControlColors.outline(), outlineWidth, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
    painter->drawRect(QRectF(halfOutlineWidth, halfOutlineWidth,
        width() - outlineWidth, height() - outlineWidth));
}
