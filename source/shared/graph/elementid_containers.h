/* Copyright © 2013-2022 Graphia Technologies Ltd.
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

#ifndef ELEMENTID_CONTAINERS_H
#define ELEMENTID_CONTAINERS_H

#include "elementid.h"

#include <unordered_set>
#include <unordered_map>
#include <functional>

template<typename T> struct ElementIdHash
{
    size_t operator()(const ElementId<T>& x) const noexcept
    {
        return static_cast<size_t>(static_cast<int>(x));
    }
};

template<typename T> using ElementIdSet = std::unordered_set<T, ElementIdHash<T>>;
template<typename K, typename V> using ElementIdMap = std::unordered_map<K, V, ElementIdHash<K>>;

using NodeIdSet = ElementIdSet<NodeId>;
using EdgeIdSet = ElementIdSet<EdgeId>;
using ComponentIdSet = ElementIdSet<ComponentId>;

template<typename V> using NodeIdMap = ElementIdMap<NodeId, V>;
template<typename V> using EdgeIdMap = ElementIdMap<EdgeId, V>;
template<typename V> using ComponentIdMap = ElementIdMap<ComponentId, V>;

template<typename Element> using ElementConditionFn = std::function<bool(const Element)>;
using NodeConditionFn = ElementConditionFn<NodeId>;
using EdgeConditionFn = ElementConditionFn<EdgeId>;

class IGraphComponent;
using ComponentConditionFn = std::function<bool(const IGraphComponent& component)>;

#endif // ELEMENTID_CONTAINERS_H
