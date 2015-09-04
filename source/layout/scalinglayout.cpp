#include "scalinglayout.h"

void ScalingLayout::executeReal(bool)
{
    for(auto nodeId : nodeIds())
        positions().set(nodeId, positions().get(nodeId) * _scale);
}
