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

#include "visualisationchannel.h"

#include "shared/utils/container.h"

void VisualisationChannel::reset()
{
    _values.clear();
    _valueIndexMap.clear();
}

void VisualisationChannel::addValue(const QString& value)
{
    _values.emplace_back(value);
    _valueIndexMap[value] = _values.size() - 1;
}

int VisualisationChannel::indexOf(const QString& value) const
{
    if(u::contains(_valueIndexMap, value))
        return static_cast<int>(_valueIndexMap.at(value));

    return -1;
}
