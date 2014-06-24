#ifndef COLLISION_H
#define COLLISION_H

#include "../graph/graph.h"
#include "../graph/graphmodel.h"
#include "layout.h"

#include <QVector3D>

#include <vector>
#include <memory>

class Collision
{
private:
    std::shared_ptr<const ReadOnlyGraph> _graph;
    std::shared_ptr<const NodeVisuals> _nodeVisuals;
    std::shared_ptr<const NodePositions> _nodePositions;
    QVector3D _offset;

public:
    Collision(std::shared_ptr<const ReadOnlyGraph> graph, std::shared_ptr<const NodeVisuals> nodeVisuals,
              std::shared_ptr<const NodePositions> nodePositions) :
        _graph(graph), _nodeVisuals(nodeVisuals), _nodePositions(nodePositions),
        _offset(0.0f, 0.0f, 0.0f)
    {}

    void setOffset(QVector3D _offset) { _offset = _offset; }

    NodeId closestNodeToLine(const std::vector<NodeId>& nodeIds, const QVector3D& point, const QVector3D& direction);
    NodeId closestNodeToLine(const QVector3D& point, const QVector3D& direction);

    void nodesIntersectingLine(const QVector3D& point, const QVector3D& direction, std::vector<NodeId>& intersectingNodeIds);
    NodeId nearestNodeIntersectingLine(const QVector3D& point, const QVector3D& direction);
};

#endif // COLLISION_H
