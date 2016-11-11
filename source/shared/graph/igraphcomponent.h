#ifndef IGRAPHCOMPONENT_H
#define IGRAPHCOMPONENT_H

#include "elementid.h"
#include "igraph.h"

#include <vector>

class IGraphComponent
{
public:
    virtual ~IGraphComponent() {};

    virtual const std::vector<NodeId>& nodeIds() const = 0;
    int numNodes() const { return static_cast<int>(nodeIds().size()); }

    virtual const std::vector<EdgeId>& edgeIds() const = 0;
    int numEdges() const { return static_cast<int>(edgeIds().size()); }

    virtual const IGraph& graph() const = 0;
};

#endif // IGRAPHCOMPONENT_H
