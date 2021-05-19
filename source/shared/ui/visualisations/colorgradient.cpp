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

#include "colorgradient.h"

#include "shared/utils/utils.h"

#include <json_helper.h>

#include <QDebug>

#include <algorithm>

ColorGradient::ColorGradient(const QString& descriptor)
{
    auto jsonDocument = parseJsonFrom(descriptor.toUtf8());

    if(jsonDocument.is_null())
    {
        qDebug() << "ColorGradient failed to parse" << descriptor;
        return;
    }

    if(!jsonDocument.is_object())
    {
        qDebug() << "ColorGradient is not an object" << descriptor;
        return;
    }

    for(const auto& i : jsonDocument.items())
    {
        auto value = std::stod(i.key());
        auto colorString = QString::fromStdString(i.value());

        _stops.emplace_back(Stop{value, colorString});
    }

    std::sort(_stops.begin(), _stops.end());
}

QColor ColorGradient::get(double value) const
{
    if(_stops.empty())
        return {};

    if(_stops.size() == 1)
        return _stops[0]._color;

    for(size_t i = 0; i < _stops.size() - 1; i++)
    {
        const auto& from = _stops[i];
        const auto& to =   _stops[i + 1];

        if(value >= from._value && value < to._value)
        {
            auto nv = u::normalise(from._value, to._value, value);
            auto const& fc = from._color;
            auto const& tc = to._color;

            auto rd = tc.redF()   - fc.redF();
            auto gd = tc.greenF() - fc.greenF();
            auto bd = tc.blueF()  - fc.blueF();

            QColor blend;
            blend.setRedF(   fc.redF()   + (nv * rd));
            blend.setGreenF( fc.greenF() + (nv * gd));
            blend.setBlueF(  fc.blueF()  + (nv * bd));
            return blend;
        }
    }

    if(value >= _stops.back()._value)
        return _stops.back()._color;

    return _stops.front()._color;
}
