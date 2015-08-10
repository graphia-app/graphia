#ifndef GRAPHARRAY_H
#define GRAPHARRAY_H

#include "graph.h"
#include "abstractcomponentmanager.h"

#include <QObject>

#include <memory>
#include <vector>
#include <mutex>

class ResizableGraphArray
{
public:
    virtual void resize(int size) = 0;
    virtual void invalidate() = 0;
};

template<typename Index, typename Element> class GraphArray : public ResizableGraphArray
{
    static_assert(std::is_nothrow_move_constructible<Element>::value,
                  "GraphArray Element needs a noexcept move constructor");

protected:
    mutable Graph* _graph;
    std::vector<Element> _array;

public:
    GraphArray(Graph& graph) :
        _graph(&graph)
    {}

    GraphArray(const GraphArray& other) :
        _graph(other._graph)
    {
        _array = other._array;
    }

    GraphArray(GraphArray&& other) :
        _graph(other._graph),
        _array(std::move(other._array))
    {}

    virtual ~GraphArray() {}

    GraphArray& operator=(const GraphArray& other)
    {
        Q_ASSERT(_graph == other._graph);
        _array = other._array;

        return *this;
    }

    GraphArray& operator=(GraphArray&& other)
    {
        Q_ASSERT(_graph == other._graph);
        _array = std::move(other._array);

        return *this;
    }

    void invalidate() { _graph = nullptr; }

    Element& operator[](Index index)
    {
        return _array[index];
    }

    const Element& operator[](Index index) const
    {
        return _array[index];
    }

    Element& at(Index index)
    {
        return _array.at(index);
    }

    const Element& at(Index index) const
    {
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
        _array.fill(value);
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

template<typename Element> class NodeArray : public GraphArray<NodeId, Element>
{
public:
    NodeArray(Graph& graph) :
        GraphArray<NodeId, Element>(graph)
    {
        this->resize(graph.nextNodeId());
        graph._nodeArrayList.insert(this);
    }

    NodeArray(const Graph& graph) : NodeArray(const_cast<Graph&>(graph)) {}

    NodeArray(const NodeArray& other) : GraphArray<NodeId, Element>(other)
    {
        this->_graph->_nodeArrayList.insert(this);
    }

    NodeArray(NodeArray&& other) : GraphArray<NodeId, Element>(std::move(other))
    {
        this->_graph->_nodeArrayList.insert(this);
    }

    NodeArray& operator=(const NodeArray& other)
    {
        GraphArray<NodeId, Element>::operator=(other);
        return *this;
    }

    NodeArray& operator=(NodeArray&& other)
    {
        GraphArray<NodeId, Element>::operator=(std::move(other));
        return *this;
    }

    ~NodeArray()
    {
        if(this->_graph != nullptr)
            this->_graph->_nodeArrayList.erase(this);
    }
};

template<typename Element> class EdgeArray : public GraphArray<EdgeId, Element>
{
public:
    EdgeArray(Graph& graph) :
        GraphArray<EdgeId, Element>(graph)
    {
        this->resize(graph.nextEdgeId());
        graph._edgeArrayList.insert(this);
    }

    EdgeArray(const Graph& graph) : EdgeArray(const_cast<Graph&>(graph)) {}

    EdgeArray(const EdgeArray& other) : GraphArray<EdgeId, Element>(other)
    {
        this->_graph->_edgeArrayList.insert(this);
    }

    EdgeArray(EdgeArray&& other) : GraphArray<EdgeId, Element>(std::move(other))
    {
        this->_graph->_edgeArrayList.insert(this);
    }

    EdgeArray& operator=(const EdgeArray& other)
    {
        GraphArray<EdgeId, Element>::operator=(other);
        return *this;
    }

    EdgeArray& operator=(EdgeArray&& other)
    {
        GraphArray<EdgeId, Element>::operator=(std::move(other));
        return *this;
    }

    ~EdgeArray()
    {
        if(this->_graph != nullptr)
            this->_graph->_edgeArrayList.erase(this);
    }
};

template<typename Element> class ComponentArray : public GraphArray<ComponentId, Element>
{
public:
    ComponentArray(Graph& graph) :
        GraphArray<ComponentId, Element>(graph)
    {
        Q_ASSERT(graph._componentManager != nullptr);
        this->resize(graph._componentManager->componentArrayCapacity());
        graph._componentManager->_componentArrayList.insert(this);
    }

    ComponentArray(const Graph& graph) : ComponentArray(const_cast<Graph&>(graph)) {}

    ComponentArray(const ComponentArray& other) : GraphArray<ComponentId, Element>(other)
    {
        Q_ASSERT(this->_graph->_componentManager != nullptr);
        this->_graph->_componentManager->_componentArrayList.insert(this);
    }

    ComponentArray(ComponentArray&& other) : GraphArray<ComponentId, Element>(std::move(other))
    {
        Q_ASSERT(this->_graph->_componentManager != nullptr);
        this->_graph->_componentManager->_componentArrayList.insert(this);
    }

    ComponentArray& operator=(const ComponentArray& other)
    {
        GraphArray<ComponentId, Element>::operator=(other);
        return *this;
    }

    ComponentArray& operator=(ComponentArray&& other)
    {
        GraphArray<ComponentId, Element>::operator=(std::move(other));
        return *this;
    }

    ~ComponentArray()
    {
        if(this->_graph != nullptr)
            this->_graph->_componentManager->_componentArrayList.erase(this);
    }
};

#endif // GRAPHARRAY_H
