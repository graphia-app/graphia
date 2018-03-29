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
