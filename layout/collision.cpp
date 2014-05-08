#include "collision.h"
#include "../maths/ray.h"
#include "../maths/plane.h"
#include "../gl/graphscene.h"

NodeId Collision::closestNodeToLine(const QList<NodeId>& nodeIds, const QVector3D &point, const QVector3D &direction)
{
    Plane plane(point, direction);
    NodeId closestNodeId = NodeId::Null();
    float minimumDistance = std::numeric_limits<float>::max();

    for(NodeId nodeId : nodeIds)
    {
        const QVector3D& position = (*_nodePositions)[nodeId] + offset;

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

void Collision::nodesIntersectingLine(const QVector3D& point, const QVector3D& direction, QList<NodeId>& intersectingNodeIds)
{
    Plane plane(point, direction);

    const QList<NodeId>& nodeIds = _graph->nodeIds();
    for(NodeId nodeId : nodeIds)
    {
        const QVector3D& position = (*_nodePositions)[nodeId] + offset;

        if(plane.sideForPoint(position) != Plane::Side::Front)
            continue;

        float distance = position.distanceToLine(point, direction);

        if(distance <= (*_nodeVisuals)[nodeId].size)
            intersectingNodeIds.append(nodeId);
    }
}

NodeId Collision::nearestNodeIntersectingLine(const QVector3D& point, const QVector3D& direction)
{
    QList<NodeId> nodeIds;

    nodesIntersectingLine(point, direction, nodeIds);

    NodeId closestNodeId = NodeId::Null();
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
