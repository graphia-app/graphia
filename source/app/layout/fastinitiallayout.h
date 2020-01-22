#ifndef FASTINITIALLAYOUT_H
#define FASTINITIALLAYOUT_H

#include "layout.h"

class FastInitialLayout : public Layout
{
    Q_OBJECT

private:
    void positionNode(QVector3D& offsetPosition, const QMatrix4x4& orientationMatrix,
                      const QVector3D& parentNodePosition, NodeId childNodeId,
                      NodeArray<QVector3D>& directionNodeVectors);
public:
    FastInitialLayout(const IGraphComponent& graphComponent, NodeLayoutPositions& positions)
        : Layout(graphComponent, positions)
    {}

    void execute(bool, Dimensionality) override;
};

#endif // FASTINITIALLAYOUT_H
