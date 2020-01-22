#include "randomlayout.h"

#include "shared/utils/random.h"

void RandomLayout::execute(bool, Dimensionality)
{
    for(auto nodeId : nodeIds())
        positions().set(nodeId, u::randQVector3D(-_spread, _spread));
}
