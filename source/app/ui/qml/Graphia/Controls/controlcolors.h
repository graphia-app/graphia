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

#ifndef CONTROLCOLORS_H
#define CONTROLCOLORS_H

#include <QQmlEngine>
#include <QGuiApplication>
#include <QObject>
#include <QColor>
#include <QPalette>

class QQmlEngine;
class QJSEngine;

class ControlColors : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_DISABLE_COPY(ControlColors)

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

    Q_PROPERTY(Qt::ColorScheme scheme READ scheme NOTIFY colorSchemeChanged)

public:
    explicit ControlColors(QObject* parent = nullptr);

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
    void colorSchemeChanged();
};

#endif // CONTROLCOLORS_H
