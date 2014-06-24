#include "centreinglayout.h"

void CentreingLayout::executeReal(uint64_t)
{
    auto& positions = *_positions;

    QVector3D centerOfMass;
    for(NodeId nodeId : _graph->nodeIds())
        centerOfMass += (positions[nodeId] / _graph->numNodes());

    positions.lock();
    for(NodeId nodeId : _graph->nodeIds())
        positions[nodeId] -= centerOfMass;
    positions.unlock();

    emit changed();
}
