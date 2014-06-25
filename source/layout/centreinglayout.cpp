#include "centreinglayout.h"

void CentreingLayout::executeReal(uint64_t)
{
    QVector3D centerOfMass;
    for(NodeId nodeId : _graph.nodeIds())
        centerOfMass += (_positions[nodeId] / _graph.numNodes());

    _positions.lock();
    for(NodeId nodeId : _graph.nodeIds())
        _positions[nodeId] -= centerOfMass;
    _positions.unlock();

    emit changed();
}
