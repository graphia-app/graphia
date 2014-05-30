#include "randomlayout.h"

#include "../utils.h"

void RandomLayout::executeReal(uint64_t)
{
    NodePositions& positions = *this->_positions;
    int nodeNumber = 0;
    int numNodes = graph().numNodes();

    positions.lock();
    for(NodeId nodeId : graph().nodeIds())
    {
        positions[nodeId] = Utils::randQVector3D(-_spread, _spread);
        int percentage = (nodeNumber++ * 100) / numNodes;
        emit progress(percentage);
    }
    positions.unlock();

    emit changed();
}
