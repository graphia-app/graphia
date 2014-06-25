#include "randomlayout.h"

#include "../utils.h"

void RandomLayout::executeReal(uint64_t)
{
    int nodeNumber = 0;
    int numNodes = _graph.numNodes();

    _positions.lock();
    for(NodeId nodeId : _graph.nodeIds())
    {
        _positions[nodeId] = Utils::randQVector3D(-_spread, _spread);
        int percentage = (nodeNumber++ * 100) / numNodes;
        emit progress(percentage);
    }
    _positions.unlock();

    emit changed();
}
