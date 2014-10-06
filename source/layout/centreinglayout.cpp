#include "centreinglayout.h"

void CentreingLayout::executeReal(uint64_t)
{
    QVector3D centreOfMass;
    float numNodesReciprocal = 1.0f / _graph.numNodes();

    for(NodeId nodeId : _graph.nodeIds())
        centreOfMass += _positions.get(nodeId) * numNodesReciprocal;

    _positions.update(_graph, [&](NodeId, const QVector3D& position)
        {
            return position - centreOfMass;
        });
}
