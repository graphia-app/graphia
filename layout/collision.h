#ifndef COLLISION_H
#define COLLISION_H

#include "../graph/graph.h"
#include "layout.h"

#include <QList>
#include <QVector3D>

class GraphScene;

class Collision
{
private:
    const ReadOnlyGraph* _graph;
    const NodePositions* _nodePositions;

public:
    Collision(const ReadOnlyGraph& graph, const NodePositions& _nodePositions) :
        _graph(&graph), _nodePositions(&_nodePositions)
    {}

    NodeId closestNodeToLine(const QList<NodeId> &nodeIds, const QVector3D& point, const QVector3D& direction);
    NodeId closestNodeToLine(const QVector3D& point, const QVector3D& direction);
};

#endif // COLLISION_H
