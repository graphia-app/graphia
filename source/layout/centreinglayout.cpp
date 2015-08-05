#include "centreinglayout.h"

void CentreingLayout::executeReal(uint64_t)
{
    QVector3D centreOfMass = NodePositions::centreOfMass(positions(), nodeIds());

    for(auto nodeId : nodeIds())
        positions().set(nodeId, positions().get(nodeId) - centreOfMass);
}
