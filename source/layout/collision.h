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
    const GraphModel& _graphModel;
    ComponentId _componentId;
    QVector3D _offset;

public:
    Collision(const GraphModel& graphModel, ComponentId componentId) :
        _graphModel(graphModel),
        _componentId(componentId),
        _offset(0.0f, 0.0f, 0.0f)
    {}

    void setOffset(QVector3D _offset) { _offset = _offset; }

    NodeId nodeClosestToLine(const std::vector<NodeId>& nodeIds, const QVector3D& point, const QVector3D& direction);
    NodeId nodeClosestToLine(const QVector3D& point, const QVector3D& direction);

    void nodesIntersectingLine(const QVector3D& point, const QVector3D& direction, std::vector<NodeId>& intersectingNodeIds);
    NodeId nearestNodeIntersectingLine(const QVector3D& point, const QVector3D& direction);
};

#endif // COLLISION_H
