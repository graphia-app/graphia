#include "scalinglayout.h"

void ScalingLayout::executeReal()
{
    NodeArray<QVector3D>& positions = *this->positions;

    positions.lock();
    for(NodeId nodeId : graph().nodeIds())
        positions[nodeId] = positions[nodeId] * _scale;
    positions.unlock();

    emit complete();
}
