#include "scalinglayout.h"

void ScalingLayout::executeReal(uint64_t)
{
    auto& positions = *_positions;

    positions.lock();
    for(NodeId nodeId : _graph->nodeIds())
        positions[nodeId] = positions[nodeId] * _scale;
    positions.unlock();

    emit changed();
}
