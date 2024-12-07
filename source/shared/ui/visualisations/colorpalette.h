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

#ifndef COLORPALETTE_H
#define COLORPALETTE_H

#include <QString>
#include <QColor>

#include <vector>
#include <map>

class ColorPalette
{
private:
    std::map<QString, QColor> _fixedColors;
    std::vector<QColor> _colors;
    QColor _defaultColor;

public:
    ColorPalette() = default;
    ColorPalette(const ColorPalette&) = default;
    ColorPalette& operator=(const ColorPalette&) = default;
    explicit ColorPalette(const QString& descriptor);

    QColor get(const QString& value, int index) const;
};

#endif // COLORPALETTE_H
