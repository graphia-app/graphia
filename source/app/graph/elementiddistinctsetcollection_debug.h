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

#ifndef ELEMENTIDDISTINCTSETCOLLECTION_DEBUG_H
#define ELEMENTIDDISTINCTSETCOLLECTION_DEBUG_H

#include "elementiddistinctsetcollection.h"

#include <QDebug>

template<typename C, typename T> QDebug operator<<(QDebug d, const ElementIdDistinctSet<C, T>& set)
{
    d << "[";
    for(auto id : set)
        d << id;
    d << "]";

    return d;
}

template<typename T> QDebug operator<<(QDebug d, const ElementIdDistinctSets<T>& set)
{
    d << "[";
    for(auto id : set)
        d << id;
    d << "]";

    return d;
}

#endif // ELEMENTIDDISTINCTSETCOLLECTION_DEBUG_H
