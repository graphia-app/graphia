#ifndef GRAPHARRAY_H
#define GRAPHARRAY_H

#include "graph.h"
#include "componentmanager.h"

#include <QObject>
#include <QMutex>
#include <QMutexLocker>

#include <memory>
#include <vector>

class ResizableGraphArray
{
public:
    virtual void resize(int size) = 0;
};

template<typename Index, typename Element> class GraphArray : public ResizableGraphArray
{
protected:
    std::shared_ptr<Graph> _graph;
    std::vector<Element> _array;
    QMutex _mutex;
    bool _flag; // Generic flag

public:
    GraphArray(std::shared_ptr<Graph> graph) :
        _graph(graph),
        _mutex(QMutex::Recursive),
        _flag(false)
    {}
    GraphArray(const GraphArray& other) :
        _graph(other._graph),
        _mutex(QMutex::Recursive),
        _flag(other._flag)
    {
        for(auto e : other._array)
            _array.push_back(e);
    }

    virtual ~GraphArray() {}

    GraphArray& operator=(const GraphArray& other)
    {
        Q_ASSERT(_graph.get() == other._graph.get());
        _array = other._array;
        _flag = other._flag;

        return *this;
    }

    QMutex& mutex() { return _mutex; }
    void lock() { _mutex.lock(); }
    void unlock() { _mutex.unlock(); }

    bool flagged() { return _flag; }
    void flag() { _flag = true; }
    void resetFlag() { _flag = false; }

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

    const std::vector<Element>& asVector() const
    {
        return _array;
    }

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
    NodeArray(std::shared_ptr<Graph> graph) :
        GraphArray<NodeId, Element>(graph)
    {
        this->resize(graph->nodeArrayCapacity());
        graph->_nodeArrayList.insert(this);
    }

    NodeArray(const NodeArray& other) : GraphArray<NodeId, Element>(other)
    {
        this->_graph->_nodeArrayList.insert(this);
    }

    ~NodeArray()
    {
        this->_graph->_nodeArrayList.erase(this);
    }
};

template<typename Element> class EdgeArray : public GraphArray<EdgeId, Element>
{
public:
    EdgeArray(std::shared_ptr<Graph> graph) :
        GraphArray<EdgeId, Element>(graph)
    {
        this->resize(graph->edgeArrayCapacity());
        graph->_edgeArrayList.insert(this);
    }

    EdgeArray(const EdgeArray& other) : GraphArray<EdgeId, Element>(other)
    {
        this->_graph->_edgeArrayList.insert(this);
    }

    ~EdgeArray()
    {
        this->_graph->_edgeArrayList.erase(this);
    }
};

template<typename Element> class ComponentArray : public GraphArray<ComponentId, Element>
{
public:
    ComponentArray(std::shared_ptr<Graph> graph) :
        GraphArray<ComponentId, Element>(graph)
    {
        this->resize(graph->_componentManager->componentArrayCapacity());
        graph->_componentManager->_componentArrayList.insert(this);
    }

    ComponentArray(const ComponentArray& other) : GraphArray<ComponentId, Element>(other)
    {
        this->_graph->_componentManager->_componentArrayList.insert(this);
    }

    ~ComponentArray()
    {
        this->_graph->_componentManager->_componentArrayList.erase(this);
    }
};

#endif // GRAPHARRAY_H
