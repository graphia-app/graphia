/* Copyright © 2013-2021 Graphia Technologies Ltd.
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

#ifndef HOVERMOUSEPASSTHROUGH_H
#define HOVERMOUSEPASSTHROUGH_H

#include "shared/utils/static_block.h"

#include <QQmlEngine>
#include <QObject>
#include <QQuickItem>
#include <QMouseEvent>

class HoverMousePassthrough : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(bool hovered MEMBER _hovered NOTIFY hoveredChanged)

private:
    bool _hovered = false;

public:
    explicit HoverMousePassthrough(QQuickItem* parent = nullptr) : QQuickItem(parent)
    {
        setAcceptHoverEvents(true);
    }

signals:
    void hoveredChanged();

protected:
    void hoverEnterEvent(QHoverEvent *event) override
    {
        event->ignore();
        _hovered = true;
        emit hoveredChanged();
    }

    void hoverLeaveEvent(QHoverEvent *event) override
    {
        event->ignore();
        _hovered = false;
        emit hoveredChanged();
    }
};

static_block
{
    qmlRegisterType<HoverMousePassthrough>(
        APP_URI, APP_MAJOR_VERSION, APP_MINOR_VERSION, "HoverMousePassthrough");
}

#endif // HOVERMOUSEPASSTHROUGH_H
