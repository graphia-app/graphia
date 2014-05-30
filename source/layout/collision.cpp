#include "collision.h"
#include "../maths/ray.h"
#include "../maths/plane.h"
#include "../gl/graphscene.h"

NodeId Collision::closestNodeToLine(const QVector<NodeId>& nodeIds, const QVector3D &point, const QVector3D &direction)
{
    Plane plane(point, direction);
    NodeId closestNodeId;
    float minimumDistance = std::numeric_limits<float>::max();

    for(NodeId nodeId : nodeIds)
    {
        const QVector3D& position = (*_nodePositions)[nodeId] + _offset;

        if(plane.sideForPoint(position) != Plane::Side::Front)
            continue;

        float distance = position.distanceToLine(point, direction);

        if(distance < minimumDistance)
        {
            minimumDistance = distance;
            closestNodeId = nodeId;
        }
    }

    return closestNodeId;
}

NodeId Collision::closestNodeToLine(const QVector3D &point, const QVector3D &direction)
{
    return closestNodeToLine(_graph->nodeIds(), point, direction);
}

void Collision::nodesIntersectingLine(const QVector3D& point, const QVector3D& direction, QVector<NodeId>& intersectingNodeIds)
{
    Plane plane(point, direction);

    const QVector<NodeId>& nodeIds = _graph->nodeIds();
    for(NodeId nodeId : nodeIds)
    {
        const QVector3D& position = (*_nodePositions)[nodeId] + _offset;

        if(plane.sideForPoint(position) != Plane::Side::Front)
            continue;

        float distance = position.distanceToLine(point, direction);

        if(distance <= (*_nodeVisuals)[nodeId]._size)
            intersectingNodeIds.append(nodeId);
    }
}

NodeId Collision::nearestNodeIntersectingLine(const QVector3D& point, const QVector3D& direction)
{
    QVector<NodeId> nodeIds;

    nodesIntersectingLine(point, direction, nodeIds);

    NodeId closestNodeId;
    float minimumDistance = std::numeric_limits<float>::max();

    for(NodeId nodeId : nodeIds)
    {
        float distance = (*_nodePositions)[nodeId].distanceToPoint(point);

        if(distance < minimumDistance)
        {
            minimumDistance = distance;
            closestNodeId = nodeId;
        }
    }

    return closestNodeId;
}
