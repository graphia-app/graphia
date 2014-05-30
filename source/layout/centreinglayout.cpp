#include "centreinglayout.h"

void CentreingLayout::executeReal(uint64_t)
{
    NodePositions& positions = *this->_positions;

    QVector3D centerOfMass;
    for(NodeId nodeId : graph().nodeIds())
        centerOfMass += (positions[nodeId] / graph().numNodes());

    positions.lock();
    for(NodeId nodeId : graph().nodeIds())
        positions[nodeId] -= centerOfMass;
    positions.unlock();

    emit changed();
}
