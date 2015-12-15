#ifndef GRAPHARRAY_H
#define GRAPHARRAY_H

#include "graph.h"
#include "../utils/utils.h"

#include <QObject>

#include <memory>
#include <vector>
#include <mutex>

class GraphArray
{
public:
    virtual void resize(int size) = 0;
    virtual void invalidate() = 0;
};

template<typename Index, typename Element, typename Locking = void>
class GenericGraphArray : public GraphArray
{
private:
    static_assert(std::is_nothrow_move_constructible<Element>::value,
                  "GraphArray Element needs a noexcept move constructor");

    using MaybeLock = u::MaybeLock<std::recursive_mutex, Locking>;

protected:
    const Graph* _graph;
    std::vector<Element> _array;
    mutable std::recursive_mutex _mutex;

public:
    GenericGraphArray(const Graph& graph) :
        _graph(&graph)
    {}

    GenericGraphArray(const Graph& graph, const Element& defaultValue) :
        _graph(&graph)
    {
        fill(defaultValue);
    }

    GenericGraphArray(const GenericGraphArray& other) :
        _graph(other._graph),
        _array(other._array)
    {}

    GenericGraphArray(GenericGraphArray&& other) :
        _graph(other._graph),
        _array(std::move(other._array))
    {}

    virtual ~GenericGraphArray() {}

    GenericGraphArray& operator=(const GenericGraphArray& other)
    {
        Q_ASSERT(_graph == other._graph);
        _array = other._array;

        return *this;
    }

    GenericGraphArray& operator=(GenericGraphArray&& other)
    {
        Q_ASSERT(_graph == other._graph);
        _array = std::move(other._array);

        return *this;
    }

    void invalidate()
    {
        MaybeLock lock(_mutex);
        _graph = nullptr;
    }

    Element& operator[](Index index)
    {
        MaybeLock lock(_mutex);
        Q_ASSERT(index >= 0 && index < size());
        return _array[index];
    }

    const Element& operator[](Index index) const
    {
        MaybeLock lock(_mutex);
        Q_ASSERT(index >= 0 && index < size());
        return _array[index];
    }

    Element& at(Index index)
    {
        MaybeLock lock(_mutex);
        Q_ASSERT(index >= 0 && index < size());
        return _array.at(index);
    }

    const Element& at(Index index) const
    {
        MaybeLock lock(_mutex);
        Q_ASSERT(index >= 0 && index < size());
        return _array.at(index);
    }

    Element get(Index index) const
    {
        MaybeLock lock(_mutex);
        Q_ASSERT(index >= 0 && index < size());
        return _array[index];
    }

    void set(Index index, const Element& value)
    {
        MaybeLock lock(_mutex);
        Q_ASSERT(index >= 0 && index < size());
        _array[index] = value;
    }

    //FIXME these iterators do not lock when locking is enabled; need to wrap in own iterator types
    typename std::vector<Element>::iterator begin() { return _array.begin(); }
    typename std::vector<Element>::const_iterator begin() const { return _array.begin(); }
    typename std::vector<Element>::iterator end() { return _array.end(); }
    typename std::vector<Element>::const_iterator end() const { return _array.end(); }

    int size() const
    {
        MaybeLock lock(_mutex);
        return static_cast<int>(_array.size());
    }

    bool empty() const
    {
        MaybeLock lock(_mutex);
        return _array.empty();
    }

    void resize(int size)
    {
        MaybeLock lock(_mutex);
        _array.resize(size);
    }

    void fill(const Element& value)
    {
        MaybeLock lock(_mutex);
        std::fill(_array.begin(), _array.end(), value);
    }

    void resetElements()
    {
        MaybeLock lock(_mutex);
        fill(Element());
    }

    void dumpToQDebug(int detail) const
    {
        qDebug() << "GraphArray size" << _array.size();

        if(detail > 0)
        {
            for(Element e : _array)
                qDebug() << e;
        }
    }
};

template<typename Element, typename Locking = void>
class NodeArray : public GenericGraphArray<NodeId, Element, Locking>
{
public:
    NodeArray(const Graph& graph) :
        GenericGraphArray<NodeId, Element, Locking>(graph)
    {
        this->resize(graph.nextNodeId());
        graph._nodeArrays.insert(this);
    }

    NodeArray(const Graph& graph, const Element& defaultValue) :
        GenericGraphArray<NodeId, Element, Locking>(graph, defaultValue)
    {
        this->resize(graph.nextNodeId());
        graph._nodeArrays.insert(this);
    }

    NodeArray(const NodeArray& other) :
        GenericGraphArray<NodeId, Element, Locking>(other)
    {
        this->_graph->_nodeArrays.insert(this);
    }

    NodeArray(NodeArray&& other) :
        GenericGraphArray<NodeId, Element, Locking>(std::move(other))
    {
        this->_graph->_nodeArrays.insert(this);
    }

    NodeArray& operator=(const NodeArray& other)
    {
        GenericGraphArray<NodeId, Element, Locking>::operator=(other);
        return *this;
    }

    NodeArray& operator=(NodeArray&& other)
    {
        GenericGraphArray<NodeId, Element, Locking>::operator=(std::move(other));
        return *this;
    }

    ~NodeArray()
    {
        if(this->_graph != nullptr)
            this->_graph->_nodeArrays.erase(this);
    }
};

template<typename Element, typename Locking = void>
class EdgeArray : public GenericGraphArray<EdgeId, Element, Locking>
{
public:
    EdgeArray(const Graph& graph) :
        GenericGraphArray<EdgeId, Element, Locking>(graph)
    {
        this->resize(graph.nextEdgeId());
        graph._edgeArrays.insert(this);
    }

    EdgeArray(const Graph& graph, const Element& defaultValue) :
        GenericGraphArray<EdgeId, Element, Locking>(graph, defaultValue)
    {
        this->resize(graph.nextEdgeId());
        graph._edgeArrays.insert(this);
    }

    EdgeArray(const EdgeArray& other) :
        GenericGraphArray<EdgeId, Element, Locking>(other)
    {
        this->_graph->_edgeArrays.insert(this);
    }

    EdgeArray(EdgeArray&& other) :
        GenericGraphArray<EdgeId, Element, Locking>(std::move(other))
    {
        this->_graph->_edgeArrays.insert(this);
    }

    EdgeArray& operator=(const EdgeArray& other)
    {
        GenericGraphArray<EdgeId, Element, Locking>::operator=(other);
        return *this;
    }

    EdgeArray& operator=(EdgeArray&& other)
    {
        GenericGraphArray<EdgeId, Element, Locking>::operator=(std::move(other));
        return *this;
    }

    ~EdgeArray()
    {
        if(this->_graph != nullptr)
            this->_graph->_edgeArrays.erase(this);
    }
};

template<typename Element, typename Locking = void>
class ComponentArray : public GenericGraphArray<ComponentId, Element, Locking>
{
public:
    ComponentArray(const Graph& graph) :
        GenericGraphArray<ComponentId, Element, Locking>(graph)
    {
        Q_ASSERT(graph._componentManager != nullptr);
        this->resize(graph.numComponentArrays());
        graph.insertComponentArray(this);
    }

    ComponentArray(const Graph& graph, const Element& defaultValue) :
        GenericGraphArray<ComponentId, Element, Locking>(graph, defaultValue)
    {
        Q_ASSERT(graph._componentManager != nullptr);
        this->resize(graph.numComponentArrays());
        graph.insertComponentArray(this);
    }

    ComponentArray(const ComponentArray& other) :
        GenericGraphArray<ComponentId, Element, Locking>(other)
    {
        Q_ASSERT(this->_graph->_componentManager != nullptr);
        this->_graph->insertComponentArray(this);
    }

    ComponentArray(ComponentArray&& other) :
        GenericGraphArray<ComponentId, Element, Locking>(std::move(other))
    {
        Q_ASSERT(this->_graph->_componentManager != nullptr);
        this->_graph->insertComponentArray(this);
    }

    ComponentArray& operator=(const ComponentArray& other)
    {
        GenericGraphArray<ComponentId, Element, Locking>::operator=(other);
        return *this;
    }

    ComponentArray& operator=(ComponentArray&& other)
    {
        GenericGraphArray<ComponentId, Element, Locking>::operator=(std::move(other));
        return *this;
    }

    ~ComponentArray()
    {
        if(this->_graph != nullptr)
            this->_graph->eraseComponentArray(this);
    }
};

#endif // GRAPHARRAY_H
