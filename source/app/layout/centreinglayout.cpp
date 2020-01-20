#include "centreinglayout.h"

void CentreingLayout::executeReal(bool)
{
    QVector3D centreOfMass = positions().centreOfMass(nodeIds());

    for(auto nodeId : nodeIds())
        positions().set(nodeId, positions().get(nodeId) - centreOfMass);
}
