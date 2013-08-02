#ifndef GRAPHARRAY_H
#define GRAPHARRAY_H

#include "graph.h"

#include <QVector>

template<typename Index, typename Element> class GraphArray
{
protected:
    Graph* _graph;
    QVector<Element> array;

public:
    GraphArray(Graph& graph) :
        _graph(&graph)
    {}

    virtual ~GraphArray() {}

    Graph* graph() { return _graph; }

    Element& operator[](Index index)
    {
        return array[index];
    }

    const Element& operator[](Index index) const
    {
        return array[index];
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
        this->array.resize(graph.nodeArrayCapactity());
        graph.addChangeListener(this);
    }

    ~NodeArray()
    {
        this->_graph->removeChangeListener(this);
    }

private:
    void onNodeAdded(Graph::NodeId)
    {
        this->array.resize(this->_graph->nodeArrayCapactity());
    }
};

template<typename Element> class EdgeArray : public GraphArray<Graph::EdgeId, Element>, public Graph::ChangeListener
{
public:
    EdgeArray(Graph& graph) : GraphArray<Graph::EdgeId, Element>(graph)
    {
        this->array.resize(graph.edgeArrayCapactity());
        graph.addChangeListener(this);
    }

    ~EdgeArray()
    {
        this->_graph->removeChangeListener(this);
    }

private:
    void onEdgeAdded(Graph::EdgeId)
    {
        this->array.resize(this->_graph->edgeArrayCapactity());
    }
};

#endif // GRAPHARRAY_H
