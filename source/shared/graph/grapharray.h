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

#ifndef GRAPHARRAY_H
#define GRAPHARRAY_H

#include "shared/graph/igrapharray.h"
#include "shared/graph/igrapharrayclient.h"

#include <vector>
#include <mutex>
#include <type_traits>
#include <cassert>
#include <algorithm>

struct LockingGraphArray {};

// A lock that does nothing if the second parameter isn't LockingGraphArray
template<typename Mutex, typename L> struct _MaybeLock
{
    explicit _MaybeLock(Mutex&) {}
};

template<typename Mutex> struct _MaybeLock<Mutex, LockingGraphArray>
{
    std::unique_lock<Mutex> _lock;
    explicit _MaybeLock(Mutex& mutex) : _lock(mutex) {}
};

template<typename Index, typename Element, typename Locking = void>
class GenericGraphArray : public IGraphArray
{
private:
    static_assert(std::is_nothrow_move_constructible_v<Element>,
                  "GraphArray Element needs a noexcept move constructor");

    using MaybeLock = _MaybeLock<std::recursive_mutex, Locking>;

protected:
    const IGraphArrayClient* _graph;
    std::vector<Element> _array;
    mutable std::recursive_mutex _mutex;
    Element _defaultValue;

    const Element& elementFor(Index index) const
    {
        assert(!index.isNull() && static_cast<size_t>(index) < size());
        return _array[static_cast<size_t>(index)];
    }

    Element& elementFor(Index index)
    {
        assert(!index.isNull() && static_cast<size_t>(index) < size());
        return _array[static_cast<size_t>(index)];
    }

public:
    explicit GenericGraphArray(const IGraphArrayClient& graph) :
        _graph(&graph), _defaultValue()
    {}

    GenericGraphArray(const IGraphArrayClient& graph, const Element& defaultValue) :
        _graph(&graph), _defaultValue(defaultValue)
    {
        fill(defaultValue);
    }

    GenericGraphArray(const GenericGraphArray& other) :
        _graph(other._graph),
        _array(other._array),
        _defaultValue(other._defaultValue)
    {}

    GenericGraphArray(GenericGraphArray&& other) noexcept :
        _graph(other._graph),
        _array(std::move(other._array)),
        _defaultValue(std::move(other._defaultValue))
    {}

    ~GenericGraphArray() override = default;

    GenericGraphArray& operator=(const GenericGraphArray& other)
    {
        if(this == &other)
            return *this;

        assert(_graph == other._graph);
        _array = other._array;
        _defaultValue = other._defaultValue;

        return *this;
    }

    GenericGraphArray& operator=(GenericGraphArray&& other) noexcept
    {
        assert(_graph == other._graph);
        _array = std::move(other._array);
        _defaultValue = std::move(other._defaultValue);

        return *this;
    }

    Element& operator[](Index index)
    {
        const MaybeLock lock(_mutex);
        return elementFor(index);
    }

    const Element& operator[](Index index) const
    {
        const MaybeLock lock(_mutex);
        return elementFor(index);
    }

    Element& at(Index index)
    {
        const MaybeLock lock(_mutex);
        return elementFor(index);
    }

    const Element& at(Index index) const
    {
        const MaybeLock lock(_mutex);
        return elementFor(index);
    }

    // get and set have to be implemented without elementFor because of std::vector<bool>
    Element get(Index index) const
    {
        const MaybeLock lock(_mutex);
        assert(!index.isNull() && static_cast<size_t>(index) < size());
        return _array[static_cast<size_t>(index)];
    }

    void set(Index index, const Element& value)
    {
        const MaybeLock lock(_mutex);
        assert(!index.isNull() && static_cast<size_t>(index) < size());
        _array[static_cast<size_t>(index)] = value;
    }

    bool operator==(const GenericGraphArray& other) const
    {
        return _graph == other._graph &&
            _array == other._array &&
            _defaultValue == other._defaultValue;
    }

    bool operator!=(const GenericGraphArray& other) const
    {
        return !operator==(other);
    }

    size_t size() const
    {
        const MaybeLock lock(_mutex);
        return _array.size();
    }

    bool empty() const
    {
        const MaybeLock lock(_mutex);
        return _array.empty();
    }

    void fill(const Element& value)
    {
        const MaybeLock lock(_mutex);
        std::fill(_array.begin(), _array.end(), value);
    }

    template<typename Generator>
    void generate(const Generator& generator)
    {
        const MaybeLock lock(_mutex);

        for(size_t i = 0; i < _array.size(); i++)
            _array[i] = generator(static_cast<int>(i));
    }


    void resetElements()
    {
        const MaybeLock lock(_mutex);
        fill(_defaultValue);
    }

protected:
    void resize(size_t size) override
    {
        const MaybeLock lock(_mutex);

        if constexpr(std::is_copy_constructible_v<Element>)
            _array.resize(size, _defaultValue);
        else
            _array.resize(size);
    }

    void invalidate() override
    {
        const MaybeLock lock(_mutex);
        _graph = nullptr;
    }
};

template<typename Element, typename Locking = void>
class NodeArray : public GenericGraphArray<NodeId, Element, Locking>
{
public:
    explicit NodeArray(const IGraphArrayClient& graph) :
        GenericGraphArray<NodeId, Element, Locking>(graph)
    {
        this->resize(static_cast<size_t>(graph.nextNodeId()));
        graph.insertNodeArray(this);
    }

    NodeArray(const IGraphArrayClient& graph, const Element& defaultValue) :
        GenericGraphArray<NodeId, Element, Locking>(graph, defaultValue)
    {
        this->resize(static_cast<size_t>(graph.nextNodeId()));
        graph.insertNodeArray(this);
    }

    NodeArray(const NodeArray& other) :
        GenericGraphArray<NodeId, Element, Locking>(other)
    {
        this->_graph->insertNodeArray(this);
    }

    NodeArray(NodeArray&& other) noexcept :
        GenericGraphArray<NodeId, Element, Locking>(std::move(other))
    {
        this->_graph->insertNodeArray(this);
    }

    NodeArray& operator=(const NodeArray& other)
    {
        if(this == &other)
            return *this;

        GenericGraphArray<NodeId, Element, Locking>::operator=(other);
        return *this;
    }

    NodeArray& operator=(NodeArray&& other) noexcept
    {
        GenericGraphArray<NodeId, Element, Locking>::operator=(std::move(other));
        return *this;
    }

    ~NodeArray() override
    {
        if(this->_graph != nullptr)
            this->_graph->eraseNodeArray(this);
    }

    typename std::vector<Element>::const_iterator begin() const
    {
        return this->_array.begin();
    }

    typename std::vector<Element>::const_iterator end() const
    {
        auto highest = static_cast<int>(this->_graph->lastNodeIdInUse());
        return begin() + (highest + 1);
    }
};

template<typename Element, typename Locking = void>
class EdgeArray : public GenericGraphArray<EdgeId, Element, Locking>
{
public:
    explicit EdgeArray(const IGraphArrayClient& graph) :
        GenericGraphArray<EdgeId, Element, Locking>(graph)
    {
        this->resize(static_cast<size_t>(graph.nextEdgeId()));
        graph.insertEdgeArray(this);
    }

    EdgeArray(const IGraphArrayClient& graph, const Element& defaultValue) :
        GenericGraphArray<EdgeId, Element, Locking>(graph, defaultValue)
    {
        this->resize(static_cast<size_t>(graph.nextEdgeId()));
        graph.insertEdgeArray(this);
    }

    EdgeArray(const EdgeArray& other) :
        GenericGraphArray<EdgeId, Element, Locking>(other)
    {
        this->_graph->insertEdgeArray(this);
    }

    EdgeArray(EdgeArray&& other) noexcept :
        GenericGraphArray<EdgeId, Element, Locking>(std::move(other))
    {
        this->_graph->insertEdgeArray(this);
    }

    EdgeArray& operator=(const EdgeArray& other)
    {
        if(this == &other)
            return *this;

        GenericGraphArray<EdgeId, Element, Locking>::operator=(other);
        return *this;
    }

    EdgeArray& operator=(EdgeArray&& other) noexcept
    {
        GenericGraphArray<EdgeId, Element, Locking>::operator=(std::move(other));
        return *this;
    }

    ~EdgeArray() override
    {
        if(this->_graph != nullptr)
            this->_graph->eraseEdgeArray(this);
    }

    typename std::vector<Element>::const_iterator begin() const
    {
        return this->_array.begin();
    }

    typename std::vector<Element>::const_iterator end() const
    {
        auto highest = static_cast<int>(this->_graph->lastEdgeIdInUse());
        return begin() + (highest + 1);
    }
};

template<typename Element, typename Locking = void>
class ComponentArray : public GenericGraphArray<ComponentId, Element, Locking>
{
public:
    explicit ComponentArray(const IGraphArrayClient& graph) :
        GenericGraphArray<ComponentId, Element, Locking>(graph)
    {
        assert(graph.isComponentManaged());
        this->resize(graph.numComponentArrays());
        graph.insertComponentArray(this);
    }

    ComponentArray(const IGraphArrayClient& graph, const Element& defaultValue) :
        GenericGraphArray<ComponentId, Element, Locking>(graph, defaultValue)
    {
        assert(graph.isComponentManaged());
        this->resize(graph.numComponentArrays());
        graph.insertComponentArray(this);
    }

    ComponentArray(const ComponentArray& other) :
        GenericGraphArray<ComponentId, Element, Locking>(other)
    {
        assert(this->_graph->isComponentManaged());
        this->_graph->insertComponentArray(this);
    }

    ComponentArray(ComponentArray&& other) noexcept :
        GenericGraphArray<ComponentId, Element, Locking>(std::move(other))
    {
        assert(this->_graph->isComponentManaged());
        this->_graph->insertComponentArray(this);
    }

    ComponentArray& operator=(const ComponentArray& other)
    {
        if(this == &other)
            return *this;

        GenericGraphArray<ComponentId, Element, Locking>::operator=(other);
        return *this;
    }

    ComponentArray& operator=(ComponentArray&& other) noexcept
    {
        GenericGraphArray<ComponentId, Element, Locking>::operator=(std::move(other));
        return *this;
    }

    ~ComponentArray() override
    {
        if(this->_graph != nullptr)
            this->_graph->eraseComponentArray(this);
    }

    typename std::vector<Element>::const_iterator begin() const { return this->_array.begin(); }
    typename std::vector<Element>::const_iterator end() const { return this->_array.end(); }
};

template<typename ElementId, typename Element>
class _ElementIdArray {};

template<typename Element>
struct _ElementIdArray<NodeId, Element> { using type = NodeArray<Element>; };

template<typename Element>
struct _ElementIdArray<EdgeId, Element> { using type = EdgeArray<Element>; };

template<typename Element>
struct _ElementIdArray<ComponentId, Element> { using type = ComponentArray<Element>; };

// Use this to generically create Node/Edge/ComponentArrays
// e.g. ElementIdArray<NodeId, Element> = NodeArray<Element>
template<typename ElementId, typename Element>
using ElementIdArray = typename _ElementIdArray<ElementId, Element>::type;

#endif // GRAPHARRAY_H
