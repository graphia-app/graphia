#ifndef GRAPHARRAY_H
#define GRAPHARRAY_H

#include "graph.h"
#include "componentmanager.h"

#include <QObject>
#include <QVector>
#include <QMutex>
#include <QMutexLocker>

class ResizableGraphArray
{
public:
    virtual void resize(int size) = 0;
};

template<typename Index, typename Element> class GraphArray : public ResizableGraphArray
{
protected:
    Graph* _graph;
    QVector<Element> array;
    QMutex _mutex;

public:
    GraphArray(Graph& graph) :
        _graph(&graph)
    {}
    GraphArray(const GraphArray& other) :
        _graph(other._graph)
    {
        for(auto e : other.array)
            array.append(e);
    }

    virtual ~GraphArray() {}

    QMutex& mutex() { return _mutex; }
    void lock() { _mutex.lock(); }
    void unlock() { _mutex.unlock(); }

    const Graph& graph() { return *_graph; }

    Element& operator[](Index index)
    {
        return array[index];
    }

    const Element& operator[](Index index) const
    {
        return array[index];
    }

    typename QVector<Element>::iterator begin() { return array.begin(); }
    typename QVector<Element>::const_iterator begin() const { return array.begin(); }
    typename QVector<Element>::iterator end() { return array.end(); }
    typename QVector<Element>::const_iterator end() const { return array.end(); }

    int size() const
    {
        return array.size();
    }

    void resize(int size)
    {
        array.resize(size);
    }

    void fill(const Element& value)
    {
        array.fill(value);
    }

    void dumpToQDebug(int detail) const
    {
        qDebug() << "GraphArray size" << array.size();

        if(detail > 0)
        {
            for(Element e : array)
                qDebug() << e;
        }
    }
};

template<typename Element> class NodeArray : public GraphArray<NodeId, Element>
{
public:
    NodeArray(Graph& graph) : GraphArray<NodeId, Element>(graph)
    {
        this->resize(graph.nodeArrayCapacity());
        graph.nodeArrayList.append(this);
    }

    NodeArray(const NodeArray& other) : GraphArray<NodeId, Element>(other)
    {
        this->_graph->nodeArrayList.append(this);
    }

    ~NodeArray()
    {
        this->_graph->nodeArrayList.removeOne(this);
    }
};

template<typename Element> class EdgeArray : public GraphArray<EdgeId, Element>
{
public:
    EdgeArray(Graph& graph) : GraphArray<EdgeId, Element>(graph)
    {
        this->resize(graph.edgeArrayCapacity());
        graph.edgeArrayList.append(this);
    }

    EdgeArray(const EdgeArray& other) : GraphArray<EdgeId, Element>(other)
    {
        this->_graph->edgeArrayList.append(this);
    }

    ~EdgeArray()
    {
        this->_graph->edgeArrayList.removeOne(this);
    }
};

template<typename Element> class ComponentArray : public GraphArray<ComponentId, Element>
{
public:
    ComponentArray(Graph& graph) : GraphArray<ComponentId, Element>(graph)
    {
        this->resize(graph.componentManager->componentArrayCapacity());
        graph.componentManager->componentArrayList.append(this);
    }

    ComponentArray(const ComponentArray& other) : GraphArray<ComponentId, Element>(other)
    {
        this->_graph->componentManager->componentArrayList.append(this);
    }

    ~ComponentArray()
    {
        this->_graph->componentManager->componentArrayList.removeOne(this);
    }
};

#endif // GRAPHARRAY_H
