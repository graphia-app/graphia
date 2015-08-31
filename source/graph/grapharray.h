#ifndef GRAPHARRAY_H
#define GRAPHARRAY_H

#include "graph.h"
#include "abstractcomponentmanager.h"

#include <QObject>

#include <memory>
#include <vector>

class GraphArray
{
public:
    virtual void resize(int size) = 0;
    virtual void invalidate() = 0;
};

template<typename Index, typename Element> class GenericGraphArray : public GraphArray
{
    static_assert(std::is_nothrow_move_constructible<Element>::value,
                  "GraphArray Element needs a noexcept move constructor");

protected:
    mutable Graph* _graph;
    std::vector<Element> _array;

public:
    GenericGraphArray(Graph& graph) :
        _graph(&graph)
    {}

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

    void invalidate() { _graph = nullptr; }

    Element& operator[](Index index)
    {
        Q_ASSERT(index >= 0 && index < size());
        return _array[index];
    }

    const Element& operator[](Index index) const
    {
        Q_ASSERT(index >= 0 && index < size());
        return _array[index];
    }

    Element& at(Index index)
    {
        Q_ASSERT(index >= 0 && index < size());
        return _array.at(index);
    }

    const Element& at(Index index) const
    {
        Q_ASSERT(index >= 0 && index < size());
        return _array.at(index);
    }

    typename std::vector<Element>::iterator begin() { return _array.begin(); }
    typename std::vector<Element>::const_iterator begin() const { return _array.begin(); }
    typename std::vector<Element>::iterator end() { return _array.end(); }
    typename std::vector<Element>::const_iterator end() const { return _array.end(); }

    int size() const
    {
        return static_cast<int>(_array.size());
    }

    void resize(int size)
    {
        _array.resize(size);
    }

    void fill(const Element& value)
    {
        std::fill(_array.begin(), _array.end(), value);
    }

    void resetElements()
    {
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

template<typename Element> class NodeArray : public GenericGraphArray<NodeId, Element>
{
public:
    NodeArray(Graph& graph) :
        GenericGraphArray<NodeId, Element>(graph)
    {
        this->resize(graph.nextNodeId());
        graph._nodeArrays.insert(this);
    }

    NodeArray(const Graph& graph) : NodeArray(const_cast<Graph&>(graph)) {}

    NodeArray(const NodeArray& other) : GenericGraphArray<NodeId, Element>(other)
    {
        this->_graph->_nodeArrays.insert(this);
    }

    NodeArray(NodeArray&& other) : GenericGraphArray<NodeId, Element>(std::move(other))
    {
        this->_graph->_nodeArrays.insert(this);
    }

    NodeArray& operator=(const NodeArray& other)
    {
        GenericGraphArray<NodeId, Element>::operator=(other);
        return *this;
    }

    NodeArray& operator=(NodeArray&& other)
    {
        GenericGraphArray<NodeId, Element>::operator=(std::move(other));
        return *this;
    }

    ~NodeArray()
    {
        if(this->_graph != nullptr)
            this->_graph->_nodeArrays.erase(this);
    }
};

template<typename Element> class EdgeArray : public GenericGraphArray<EdgeId, Element>
{
public:
    EdgeArray(Graph& graph) :
        GenericGraphArray<EdgeId, Element>(graph)
    {
        this->resize(graph.nextEdgeId());
        graph._edgeArrays.insert(this);
    }

    EdgeArray(const Graph& graph) : EdgeArray(const_cast<Graph&>(graph)) {}

    EdgeArray(const EdgeArray& other) : GenericGraphArray<EdgeId, Element>(other)
    {
        this->_graph->_edgeArrays.insert(this);
    }

    EdgeArray(EdgeArray&& other) : GenericGraphArray<EdgeId, Element>(std::move(other))
    {
        this->_graph->_edgeArrays.insert(this);
    }

    EdgeArray& operator=(const EdgeArray& other)
    {
        GenericGraphArray<EdgeId, Element>::operator=(other);
        return *this;
    }

    EdgeArray& operator=(EdgeArray&& other)
    {
        GenericGraphArray<EdgeId, Element>::operator=(std::move(other));
        return *this;
    }

    ~EdgeArray()
    {
        if(this->_graph != nullptr)
            this->_graph->_edgeArrays.erase(this);
    }
};

template<typename Element> class ComponentArray : public GenericGraphArray<ComponentId, Element>
{
public:
    ComponentArray(Graph& graph) :
        GenericGraphArray<ComponentId, Element>(graph)
    {
        Q_ASSERT(graph._componentManager != nullptr);
        this->resize(graph._componentManager->componentArrayCapacity());
        graph._componentManager->_componentArrays.insert(this);
    }

    ComponentArray(const Graph& graph) : ComponentArray(const_cast<Graph&>(graph)) {}

    ComponentArray(const ComponentArray& other) : GenericGraphArray<ComponentId, Element>(other)
    {
        Q_ASSERT(this->_graph->_componentManager != nullptr);
        this->_graph->_componentManager->_componentArrays.insert(this);
    }

    ComponentArray(ComponentArray&& other) : GenericGraphArray<ComponentId, Element>(std::move(other))
    {
        Q_ASSERT(this->_graph->_componentManager != nullptr);
        this->_graph->_componentManager->_componentArrays.insert(this);
    }

    ComponentArray& operator=(const ComponentArray& other)
    {
        GenericGraphArray<ComponentId, Element>::operator=(other);
        return *this;
    }

    ComponentArray& operator=(ComponentArray&& other)
    {
        GenericGraphArray<ComponentId, Element>::operator=(std::move(other));
        return *this;
    }

    ~ComponentArray()
    {
        if(this->_graph != nullptr)
            this->_graph->_componentManager->_componentArrays.erase(this);
    }
};

#endif // GRAPHARRAY_H
