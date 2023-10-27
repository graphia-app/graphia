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

#include "qmlcontrolcolors.h"

#include "shared/utils/static_block.h"

#include <QGuiApplication>
#include <QQmlEngine>
#include <QStyleHints>

QmlControlColors::QmlControlColors(QObject* parent) :
    QObject(parent)
{
    QCoreApplication::instance()->installEventFilter(this);

    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged,
        this, &QmlControlColors::paletteChanged);

    connect(this, &QmlControlColors::paletteChanged, [this]
    {
        _palette = QGuiApplication::palette();
        _palette.setCurrentColorGroup(QPalette::ColorGroup::Active);
    });
}

QObject* QmlControlColors::qmlInstance(QQmlEngine*, QJSEngine*)
{
    return new QmlControlColors;
}

bool QmlControlColors::eventFilter(QObject* watched, QEvent* event)
{
    const bool watchedQObjectIsApplication = (watched == QCoreApplication::instance());

    if(watchedQObjectIsApplication && event->type() == QEvent::ApplicationPaletteChange)
        emit paletteChanged();

    return QObject::eventFilter(watched, event);
}

QColor QmlControlColors::outline() const
{
    auto colorScheme = QGuiApplication::styleHints()->colorScheme();

    return colorScheme == Qt::ColorScheme::Light ?
        _palette.color(QPalette::Mid) : _palette.color(QPalette::Dark);
}

QColor QmlControlColors::background() const
{
    return _palette.color(QPalette::Light);
}

QColor QmlControlColors::tableRow1() const
{
    return _palette.color(QPalette::Light);
}

QColor QmlControlColors::tableRow2() const
{
    return _palette.color(QPalette::Button);
}

static_block
{
    qmlRegisterSingletonType<QmlControlColors>(
        APP_URI, APP_MAJOR_VERSION, APP_MINOR_VERSION, "ControlColors", &QmlControlColors::qmlInstance);
}

