#include "scalinglayout.h"

void ScalingLayout::execute(bool)
{
    for(auto nodeId : nodeIds())
        positions().set(nodeId, positions().get(nodeId) * _scale);
}
