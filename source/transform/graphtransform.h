#ifndef GRAPHTRANSFORM_H
#define GRAPHTRANSFORM_H

#include "../graph/graph.h"

class GraphTransform
{
public:
    virtual bool nodeIsFiltered(const Node& node) const = 0;
    virtual bool edgeIsFiltered(const Edge& edge) const = 0;

    // This should be implemented to perform arbitrary transforms beyond what simple
    // filtering can achieve. Note in this case the transform is responsible for using
    // sensible ElementIds and sequences of adds and removes; though this is probably
    // only necessary if it's intended that the transform is component managed and
    // visualised.
    virtual void transform(const Graph& /*source*/, MutableGraph& /*target*/) const {}
};

#endif // GRAPHTRANSFORM_H
