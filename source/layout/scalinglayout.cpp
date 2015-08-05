#include "scalinglayout.h"

void ScalingLayout::executeReal(uint64_t)
{
    for(auto nodeId : nodeIds())
        positions().set(nodeId, positions().get(nodeId) * _scale);
}
