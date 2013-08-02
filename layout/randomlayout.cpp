#include "randomlayout.h"

#include "../utils.h"

void RandomLayout::execute()
{
    NodeArray<QVector3D>& positions = *this->positions;
    int nodeNumber = 0;
    int numNodes = graph().numNodes();

    for(Graph::NodeId nodeId : graph().nodeIds())
    {
        positions[nodeId] = Utils::randQVector3D(-1.0, 1.0);
        int percentage = nodeNumber++ / numNodes;
        notifyListenersOfProgress(percentage);
    }

    notifyListenersOfCompletion();
}
