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

#include "iconitem.h"

#include <QPainter>
#include <QQuickWindow>

IconItem::IconItem(QQuickItem* parent) : QQuickPaintedItem(parent)
{
    // Default size
    setWidth(24.0);
    setHeight(24.0);

    connect(this, &IconItem::enabledChanged, [this] { update(); });
    connect(this, &IconItem::iconNameChanged, [this] { update(); });
    connect(this, &IconItem::onChanged, [this] { update(); });
    connect(this, &IconItem::selectedChanged, [this] { update(); });
}

void IconItem::paint(QPainter* painter)
{
    auto mode = _selected ? QIcon::Selected : QIcon::Normal;
    auto size = painter->viewport().size();

    auto pixmap = _icon.pixmap(this->size().toSize(),
        isEnabled() ? mode : QIcon::Disabled,
        _on ? QIcon::On : QIcon::Off);

    auto x = (size.width() - pixmap.width()) / 2;
    auto y = (size.height() - pixmap.height()) / 2;

    pixmap.setDevicePixelRatio(window()->devicePixelRatio());
    painter->drawPixmap(x, y, pixmap);

    //FIXME: there is clearly still some scaling going on here at non-1.0 DPRs, which
    // appears like it's a scale up by the DPR, then a scale down by 1/DPR, so the
    // image looks dimensionally correct, but has lost some fidelity in the process
}

void IconItem::setIconName(const QString& iconName)
{
    const bool wasValid = valid();

    _iconName = iconName;
    _icon = QIcon::fromTheme(iconName);

    if(wasValid != valid())
        emit validChanged();

    emit iconNameChanged();
}
