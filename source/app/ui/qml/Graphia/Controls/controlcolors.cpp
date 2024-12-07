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

#include "controlcolors.h"

#include "shared/utils/shadedcolors.h"

#include <QStyleHints>

ControlColors::ControlColors(QObject* parent) :
    QObject(parent)
{
    QCoreApplication::instance()->installEventFilter(this);

    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this,
        [this] { emit colorSchemeChanged(); });

    connect(this, &ControlColors::paletteChanged, [this]
    {
        _palette = QGuiApplication::palette();
        _palette.setCurrentColorGroup(QPalette::ColorGroup::Active);
    });
}

bool ControlColors::eventFilter(QObject* watched, QEvent* event)
{
    const bool watchedQObjectIsApplication = (watched == QCoreApplication::instance());

    if(watchedQObjectIsApplication && event->type() == QEvent::ApplicationPaletteChange)
        emit paletteChanged();

    return QObject::eventFilter(watched, event);
}

QColor ControlColors::outline() const    { return mid(); }
QColor ControlColors::background() const { return light(); }
QColor ControlColors::tableRow1() const  { return light(); }
QColor ControlColors::tableRow2() const  { return midlight(); }

QColor ControlColors::light() const      { return ShadedColors::light(_palette); }
QColor ControlColors::neutral() const    { return ShadedColors::neutral(_palette); }
QColor ControlColors::midlight() const   { return ShadedColors::midlight(_palette); }
QColor ControlColors::mid() const        { return ShadedColors::mid(_palette); }
QColor ControlColors::dark() const       { return ShadedColors::dark(_palette); }
QColor ControlColors::shadow() const     { return ShadedColors::shadow(_palette); }
QColor ControlColors::darkest() const    { return ShadedColors::darkest(_palette); }

Qt::ColorScheme ControlColors::scheme() const
{
    return QGuiApplication::styleHints()->colorScheme();
}
