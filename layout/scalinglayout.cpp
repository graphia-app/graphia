#include "scalinglayout.h"

void ScalingLayout::executeReal()
{
    NodePositions& positions = *this->positions;

    positions.lock();
    for(NodeId nodeId : graph().nodeIds())
        positions[nodeId] = positions[nodeId] * _scale;
    positions.unlock();

    emit changed();
}
