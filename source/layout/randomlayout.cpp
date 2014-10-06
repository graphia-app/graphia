#include "randomlayout.h"

#include "../utils/utils.h"

void RandomLayout::executeReal(uint64_t)
{
    int nodeNumber = 0;
    int numNodes = _graph.numNodes();

    _positions.update(_graph, [&](NodeId, const QVector3D&)
        {
            int percentage = (nodeNumber++ * 100) / numNodes;
            emit progress(percentage);
            return Utils::randQVector3D(-_spread, _spread);
        });
}
