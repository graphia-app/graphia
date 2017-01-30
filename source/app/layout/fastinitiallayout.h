#ifndef FASTINITIALLAYOUT_H
#define FASTINITIALLAYOUT_H

#include "layout.h"

class FastInitialLayout : public Layout
{
private:
    const double SPHERE_RADIUS = 20.0;
    void positionNode(QVector3D& offsetPosition, const QMatrix4x4& orientationMatrix,
                      const QVector3D& parentNodePosition, NodeId childNodeId,
                      NodeArray<QVector3D>& directionNodeVectors);
public:
    FastInitialLayout(const GraphComponent& graphComponent, NodePositions& positions)
        : Layout(graphComponent, positions)
    {}

    void executeReal(bool);
};

#endif // FASTINITIALLAYOUT_H
