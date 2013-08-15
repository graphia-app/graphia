#ifndef GRAPHARRAY_H
#define GRAPHARRAY_H

#include "graph.h"

#include <QVector>
#include <QMutex>

template<typename Index, typename Element> class GraphArray
{
protected:
    Graph* _graph;
    QVector<Element> array;
    QMutex _mutex;

public:
    GraphArray(Graph& graph) :
        _graph(&graph)
    {}

    virtual ~GraphArray() {}

    QMutex& mutex() { return _mutex; }

    Graph* graph() { return _graph; }

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

template<typename Element> class NodeArray : public GraphArray<Graph::NodeId, Element>, public Graph::ChangeListener
{
public:
    NodeArray(Graph& graph) : GraphArray<Graph::NodeId, Element>(graph)
    {
        this->array.resize(graph.nodeArrayCapacity());
        graph.addChangeListener(this);
    }

    ~NodeArray()
    {
        this->_graph->removeChangeListener(this);
    }

private:
    void onNodeAdded(Graph::NodeId)
    {
        this->array.resize(this->_graph->nodeArrayCapacity());
    }
};

template<typename Element> class EdgeArray : public GraphArray<Graph::EdgeId, Element>, public Graph::ChangeListener
{
public:
    EdgeArray(Graph& graph) : GraphArray<Graph::EdgeId, Element>(graph)
    {
        this->array.resize(graph.edgeArrayCapacity());
        graph.addChangeListener(this);
    }

    ~EdgeArray()
    {
        this->_graph->removeChangeListener(this);
    }

private:
    void onEdgeAdded(Graph::EdgeId)
    {
        this->array.resize(this->_graph->edgeArrayCapacity());
    }
};

#endif // GRAPHARRAY_H
