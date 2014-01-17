#ifndef COLLISION_H
#define COLLISION_H

#include "../graph/graph.h"
#include "../graph/graphmodel.h"
#include "layout.h"

#include <QList>
#include <QVector3D>

class Collision
{
private:
    const ReadOnlyGraph* _graph;
    const NodeVisuals* _nodeVisuals;
    const NodePositions* _nodePositions;
    QVector3D offset;

public:
    Collision(const ReadOnlyGraph& graph, const NodeVisuals& nodeVisuals,
              const NodePositions& nodePositions) :
        _graph(&graph), _nodeVisuals(&nodeVisuals), _nodePositions(&nodePositions),
        offset(0.0f, 0.0f, 0.0f)
    {}

    void setOffset(QVector3D _offset) { offset = _offset; }

    NodeId closestNodeToLine(const QList<NodeId> &nodeIds, const QVector3D& point, const QVector3D& direction);
    NodeId closestNodeToLine(const QVector3D& point, const QVector3D& direction);

    void nodesIntersectingLine(const QVector3D& point, const QVector3D& direction, QList<NodeId>& intersectingNodeIds);
    NodeId nearestNodeIntersectingLine(const QVector3D& point, const QVector3D& direction);
};

#endif // COLLISION_H
