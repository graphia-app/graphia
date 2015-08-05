#include "randomlayout.h"

#include "../utils/utils.h"

void RandomLayout::executeReal(uint64_t)
{
    for(auto nodeId : nodeIds())
        positions().set(nodeId, Utils::randQVector3D(-_spread, _spread));
}
