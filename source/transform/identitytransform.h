#ifndef IDENTITYTRANSFORM_H
#define IDENTITYTRANSFORM_H

#include "graphtransform.h"

class IdentityTransform : public GraphTransform
{
    Q_OBJECT

public:
    IdentityTransform(const Graph& graph) :
        _graph(&graph)
    {}

    const Graph& graph() { return *_graph; }

private:
    const Graph* _graph;
};

#endif // IDENTITYTRANSFORM_H
