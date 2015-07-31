#ifndef IDENTITYTRANSFORM_H
#define IDENTITYTRANSFORM_H

#include "graphtransform.h"

class IdentityTransform : public GraphTransform
{
    Q_OBJECT

public:
    IdentityTransform(Graph& graph) :
        _graph(&graph)
    {}

    Graph& graph() { return *_graph; }

private:
    Graph* _graph;
};

#endif // IDENTITYTRANSFORM_H
