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

#ifndef ELEMENTID_DEBUG_H
#define ELEMENTID_DEBUG_H

#include "elementid.h"
#include "elementid_containers.h"

#include <QDebug>
#include <QString>

using namespace Qt::Literals::StringLiterals;

template<typename T> QDebug operator<<(QDebug d, const ElementId<T>& id)
{
    if(id.isNull())
        d << u"Null"_s;
    else
        d << static_cast<int>(id);

    return d;
}

template<typename T> QDebug qDebugIdCollection(QDebug d, const T& collection)
{
    d << "[";
    for(auto id : collection)
        d << id;
    d << "]";

    return d;
}

template<typename T> QDebug operator<<(QDebug d, const ElementIdSet<T>& set)        { return qDebugIdCollection(std::move(d), set); }
template<typename T> QDebug operator<<(QDebug d, const ElementIdHashSet<T>& set)    { return qDebugIdCollection(std::move(d), set); }

inline QDebug operator<<(QDebug d, const std::vector<NodeId>& v)        { return qDebugIdCollection(std::move(d), v); }
inline QDebug operator<<(QDebug d, const std::vector<EdgeId>& v)        { return qDebugIdCollection(std::move(d), v); }
inline QDebug operator<<(QDebug d, const std::vector<ComponentId>& v)   { return qDebugIdCollection(std::move(d), v); }

template<typename T> QDebug qDebugIdMap(QDebug d, const T& map)
{
    for(const auto& [id, value] : map)
        d << id << ":" << value;

    return d;
}

template<typename K, typename V> QDebug operator<<(QDebug d, const ElementIdMap<K, V>& map)     { return qDebugIdMap(std::move(d), map); }
template<typename K, typename V> QDebug operator<<(QDebug d, const ElementIdHashMap<K, V>& map) { return qDebugIdMap(std::move(d), map); }

#endif // ELEMENTID_DEBUG_H
