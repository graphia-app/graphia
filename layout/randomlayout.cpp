#include "randomlayout.h"

#include "../utils.h"

void RandomLayout::execute()
{
    NodeArray<QVector3D>& positions = *this->positions;
    int nodeNumber = 0;
    int numNodes = graph().numNodes();

    positions.mutex().lock();
    for(Graph::NodeId nodeId : graph().nodeIds())
    {
        positions[nodeId] = Utils::randQVector3D(-1.0, 1.0);
        int percentage = (nodeNumber++ * 100) / numNodes;
        emit progress(percentage);
    }
    positions.mutex().unlock();

    emit complete();
}
