/* Copyright © 2013-2024 Graphia Technologies Ltd.
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

#include "visualisationmapping.h"

#include "shared/utils/container.h"
#include "shared/utils/statistics.h"
#include "shared/utils/utils.h"

#include <json_helper.h>

#include <algorithm>
#include <cmath>

#include <QDebug>

using namespace Qt::Literals::StringLiterals;

VisualisationMapping::VisualisationMapping(const u::Statistics& statistics,
    const QString& descriptor) :
    _min(statistics._min), _max(statistics._max)
{
    if(descriptor.isEmpty())
    {
        // Default 1:1 mapping
        return;
    }

    auto jsonDocument = parseJsonFrom(descriptor.toUtf8());

    if(jsonDocument.is_null())
    {
        qDebug() << "VisualisationMapping failed to parse" << descriptor;
        return;
    }

    if(!jsonDocument.is_object())
    {
        qDebug() << "VisualisationMapping is not an object" << descriptor;
        return;
    }

    if(u::contains(jsonDocument, "min") && u::contains(jsonDocument, "max"))
    {
        _min = jsonDocument["min"].get<double>();
        _max = jsonDocument["max"].get<double>();
    }

    if(u::contains(jsonDocument, "type"))
    {
        auto type = jsonDocument["type"];

        if(!type.is_string())
        {
            qDebug() << "VisualisationMapping.type is not a string" << descriptor;
            return;
        }

        auto typeString = QString::fromStdString(type);

        if(typeString.compare(u"minmax"_s, Qt::CaseInsensitive) == 0)
        {
            _min = statistics._min;
            _max = statistics._max;
        }
        else if(typeString.compare(u"stddev"_s, Qt::CaseInsensitive) == 0)
        {
            _min = statistics._mean - statistics._stddev;
            _max = statistics._mean + statistics._stddev;
        }
    }

    // Clamp the min and max to the values actually present in the sample
    _min = std::clamp(_min, statistics._min, statistics._max);
    _max = std::clamp(_max, statistics._min, statistics._max);

    if(_min == _max)
    {
        // We can't map to no range, so just use the min and max of the sample
        _min = statistics._min;
        _max = statistics._max;
    }

    if(u::contains(jsonDocument, "exponent"))
        _exponent = jsonDocument["exponent"].get<double>();
}

double VisualisationMapping::map(double value) const
{
    value = u::normalise(_min, _max, value);
    value = std::clamp(value, 0.0, 1.0);
    value = std::pow(value, _exponent);

    return value;
}
