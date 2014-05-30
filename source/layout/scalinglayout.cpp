#include "scalinglayout.h"

void ScalingLayout::executeReal(uint64_t)
{
    NodePositions& positions = *this->_positions;

    positions.lock();
    for(NodeId nodeId : graph().nodeIds())
        positions[nodeId] = positions[nodeId] * _scale;
    positions.unlock();

    emit changed();
}
