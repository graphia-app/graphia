#ifndef GRAPHARRAY_H
#define GRAPHARRAY_H

#include "graph.h"

#include <QObject>
#include <QVector>
#include <QMutex>
#include <QMutexLocker>

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

class NodeArrayQObject : public QObject
{
    Q_OBJECT
private:
    virtual void onNodeAddedReal(Graph&, Graph::NodeId) = 0;

private slots:
    void onNodeAdded(Graph& graph, Graph::NodeId nodeId)
    {
        onNodeAddedReal(graph, nodeId);
    }
};

template<typename Element> class NodeArray : public NodeArrayQObject, public GraphArray<Graph::NodeId, Element>
{
public:
    NodeArray(Graph& graph) : GraphArray<Graph::NodeId, Element>(graph)
    {
        this->array.resize(graph.nodeArrayCapacity());
        this->connect(&graph, &Graph::nodeAdded, this, &NodeArray::onNodeAdded, Qt::DirectConnection);
    }

private:
    void onNodeAddedReal(Graph&, Graph::NodeId)
    {
        QMutexLocker locker(&this->mutex());
        this->array.resize(this->_graph->nodeArrayCapacity());
    }
};

class EdgeArrayQObject : public QObject
{
    Q_OBJECT
private:
    virtual void onEdgeAddedReal(Graph&, Graph::EdgeId) = 0;

private slots:
    void onEdgeAdded(Graph& graph, Graph::EdgeId edgeId)
    {
        onEdgeAddedReal(graph, edgeId);
    }
};

template<typename Element> class EdgeArray : public EdgeArrayQObject, public GraphArray<Graph::EdgeId, Element>
{
public:
    EdgeArray(Graph& graph) : GraphArray<Graph::EdgeId, Element>(graph)
    {
        this->array.resize(graph.edgeArrayCapacity());
        this->connect(&graph, &Graph::edgeAdded, this, &EdgeArray::onEdgeAdded, Qt::DirectConnection);
    }

private:
    void onEdgeAddedReal(Graph&, Graph::EdgeId)
    {
        QMutexLocker locker(&this->mutex());
        this->array.resize(this->_graph->edgeArrayCapacity());
    }
};

#endif // GRAPHARRAY_H
