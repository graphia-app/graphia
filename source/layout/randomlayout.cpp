#include "randomlayout.h"

#include "../utils.h"

void RandomLayout::executeReal(uint64_t)
{
    NodePositions& positions = *this->positions;
    int nodeNumber = 0;
    int numNodes = graph().numNodes();

    positions.lock();
    for(NodeId nodeId : graph().nodeIds())
    {
        positions[nodeId] = Utils::randQVector3D(-spread, spread);
        int percentage = (nodeNumber++ * 100) / numNodes;
        emit progress(percentage);
    }
    positions.unlock();

    emit changed();
}
