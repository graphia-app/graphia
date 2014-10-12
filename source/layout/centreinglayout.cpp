#include "centreinglayout.h"

void CentreingLayout::executeReal(uint64_t)
{
    QVector3D centreOfMass = NodePositions::centreOfMass(_positions, _graph.nodeIds());

    _positions.update(_graph, [&](NodeId, const QVector3D& position)
        {
            return position - centreOfMass;
        });
}
