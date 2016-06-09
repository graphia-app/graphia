#include "randomlayout.h"

#include "shared/utils/utils.h"

void RandomLayout::executeReal(bool)
{
    for(auto nodeId : nodeIds())
        positions().set(nodeId, u::randQVector3D(-_spread, _spread));
}
