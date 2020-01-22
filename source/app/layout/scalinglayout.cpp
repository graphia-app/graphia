#include "scalinglayout.h"

void ScalingLayout::execute(bool, Dimensionality)
{
    for(auto nodeId : nodeIds())
        positions().set(nodeId, positions().get(nodeId) * _scale);
}
