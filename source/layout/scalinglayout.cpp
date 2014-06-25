#include "scalinglayout.h"

void ScalingLayout::executeReal(uint64_t)
{
    _positions.lock();
    for(NodeId nodeId : _graph.nodeIds())
        _positions[nodeId] = _positions[nodeId] * _scale;
    _positions.unlock();

    emit changed();
}
