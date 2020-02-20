/* Copyright © 2013-2020 Graphia Technologies Ltd.
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

#include "columnannotation.h"

ColumnAnnotation::ColumnAnnotation(QString name, std::vector<QString> values) :
    _name(std::move(name)), _values(std::move(values))
{
    for(const auto& value : _values)
        _uniqueValues.emplace(value);
}

ColumnAnnotation::ColumnAnnotation(QString name,
    const ColumnAnnotation::Iterator& begin,
    const ColumnAnnotation::Iterator& end) :
    _name(std::move(name))
{
    for(auto it = begin; it != end; ++it)
    {
        _values.emplace_back(*it);
        _uniqueValues.emplace(*it);
    }
}
