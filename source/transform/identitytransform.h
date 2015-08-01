#ifndef IDENTITYTRANSFORM_H
#define IDENTITYTRANSFORM_H

#include "graphtransform.h"

class IdentityTransform : public GraphTransform
{
    Q_OBJECT

public:
    IdentityTransform(MutableGraph& graph) :
        _graph(&graph)
    {}

private:
    MutableGraph* _graph;

    void onGraphWillChange(const Graph*) { _graph->beginTransaction(); }

    void onNodeAdded(const Graph*, const Node* node) { _graph->addNode(*node); }
    void onNodeWillBeRemoved(const Graph*, const Node* node) { _graph->removeNode(node->id()); }
    void onEdgeAdded(const Graph*, const Edge* edge) { _graph->addEdge(*edge); }
    void onEdgeWillBeRemoved(const Graph*, const Edge* edge) { _graph->removeEdge(edge->id()); }

    void onGraphChanged(const Graph*) { _graph->endTransaction(); }
};

#endif // IDENTITYTRANSFORM_H
