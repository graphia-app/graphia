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

#include "namedicon.h"

#include <QPainter>
#include <QQuickWindow>

NamedIcon::NamedIcon(QQuickItem* parent) : QQuickPaintedItem(parent)
{
    // Default size
    setWidth(24.0);
    setHeight(24.0);

    connect(this, &NamedIcon::enabledChanged, [this] { update(); });
    connect(this, &NamedIcon::iconNameChanged, [this] { update(); });
    connect(this, &NamedIcon::onChanged, [this] { update(); });
    connect(this, &NamedIcon::selectedChanged, [this] { update(); });
    connect(this, &NamedIcon::fillChanged, [this] { update(); });
}

void NamedIcon::paint(QPainter* painter)
{
    auto mode = _selected ? QIcon::Selected : QIcon::Normal;
    auto size = painter->viewport().size();

    auto pixmap = _icon.pixmap(this->size().toSize(),
        isEnabled() ? mode : QIcon::Disabled,
        _on ? QIcon::On : QIcon::Off);

    pixmap.setDevicePixelRatio(window()->devicePixelRatio());

    if(!_fill)
    {
        auto x = static_cast<double>((size.width() - pixmap.width())) / (2.0 * window()->devicePixelRatio());
        auto y = static_cast<double>((size.height() - pixmap.height())) / (2.0 * window()->devicePixelRatio());

        painter->drawPixmap(static_cast<int>(x), static_cast<int>(y), pixmap);
    }
    else
    {
        if(boundingRect().size() != pixmap.size().toSizeF())
            painter->setRenderHint(QPainter::SmoothPixmapTransform);

        painter->drawPixmap(boundingRect().toRect(), pixmap);
    }

    //FIXME: there is clearly still some scaling going on here at non-1.0 DPRs, which
    // appears like it's a scale up by the DPR, then a scale down by 1/DPR, so the
    // image looks dimensionally correct, but has lost some fidelity in the process
}

void NamedIcon::setIconName(const QString& iconName)
{
    const bool wasValid = valid();

    _iconName = iconName;
    _icon = QIcon::fromTheme(iconName);

    if(wasValid != valid())
        emit validChanged();

    emit iconNameChanged();
}
