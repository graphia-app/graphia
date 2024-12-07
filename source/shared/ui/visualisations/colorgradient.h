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

#ifndef COLORGRADIENT_H
#define COLORGRADIENT_H

#include <QString>
#include <QColor>

#include <vector>

class ColorGradient
{
private:
    struct Stop
    {
        double _value;
        QColor _color;

        bool operator<(const Stop& other) const
        {
            if(_value < other._value)
                return true;

            if(_value == other._value)
                return _color.rgb() < other._color.rgb();

            return false;
        }
    };

    std::vector<Stop> _stops;

public:
    ColorGradient() = default;
    ColorGradient(const ColorGradient&) = default;
    ColorGradient& operator=(const ColorGradient&) = default;
    explicit ColorGradient(const QString& descriptor);

    QColor get(double value) const;
};

#endif // COLORGRADIENT_H
