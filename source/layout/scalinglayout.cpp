#include "scalinglayout.h"

void ScalingLayout::executeReal(uint64_t)
{
    _positions.update(_graph, [this](NodeId, const QVector3D& position)
        {
            return position * _scale;
        });
}
