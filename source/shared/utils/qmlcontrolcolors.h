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

#ifndef QMLCONTROLCOLORS_H
#define QMLCONTROLCOLORS_H

#include "shared/utils/static_block.h"

#include <QQmlEngine>
#include <QGuiApplication>
#include <QObject>
#include <QColor>
#include <QPalette>

class QQmlEngine;
class QJSEngine;

class QmlControlColors : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(QmlControlColors)

    Q_PROPERTY(QColor outline READ outline NOTIFY paletteChanged)
    Q_PROPERTY(QColor background READ background NOTIFY paletteChanged)
    Q_PROPERTY(QColor tableRow1 READ tableRow1 NOTIFY paletteChanged)
    Q_PROPERTY(QColor tableRow2 READ tableRow2 NOTIFY paletteChanged)

    Q_PROPERTY(QColor light READ light NOTIFY paletteChanged)
    Q_PROPERTY(QColor neutral READ neutral NOTIFY paletteChanged)
    Q_PROPERTY(QColor midlight READ midlight NOTIFY paletteChanged)
    Q_PROPERTY(QColor mid READ mid NOTIFY paletteChanged)
    Q_PROPERTY(QColor dark READ dark NOTIFY paletteChanged)
    Q_PROPERTY(QColor shadow READ shadow NOTIFY paletteChanged)
    Q_PROPERTY(QColor darkest READ darkest NOTIFY paletteChanged)

    Q_PROPERTY(Qt::ColorScheme scheme READ scheme NOTIFY paletteChanged)

public:
    explicit QmlControlColors(QObject* parent = nullptr);
    static QObject* qmlInstance(QQmlEngine*, QJSEngine*);

    QColor outline() const;
    QColor background() const;
    QColor tableRow1() const;
    QColor tableRow2() const;

    QColor light() const;
    QColor neutral() const;
    QColor midlight() const;
    QColor mid() const;
    QColor dark() const;
    QColor shadow() const;
    QColor darkest() const;

    Qt::ColorScheme scheme() const;

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    QPalette _palette;

signals:
    void paletteChanged();
};

static_block
{
    qmlRegisterSingletonType<QmlControlColors>(
        APP_URI, APP_MAJOR_VERSION, APP_MINOR_VERSION, "ControlColors", &QmlControlColors::qmlInstance);
}

#endif // QMLCONTROLCOLORS_H
