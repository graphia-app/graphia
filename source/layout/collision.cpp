#include "collision.h"
#include "../maths/ray.h"
#include "../maths/plane.h"
#include "../rendering/graphcomponentscene.h"

NodeId Collision::nodeClosestToLine(const std::vector<NodeId>& nodeIds, const QVector3D &point, const QVector3D &direction)
{
    Plane plane(point, direction);
    NodeId closestNodeId;
    float minimumDistance = std::numeric_limits<float>::max();

    for(NodeId nodeId : nodeIds)
    {
        const QVector3D position = _graphModel.nodePositions().getScaledAndSmoothed(nodeId) + _offset;

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

NodeId Collision::nodeClosestToLine(const QVector3D &point, const QVector3D &direction)
{
    return nodeClosestToLine(_graphModel.graph().componentById(_componentId)->nodeIds(), point, direction);
}

void Collision::nodesIntersectingLine(const QVector3D& point, const QVector3D& direction, std::vector<NodeId>& intersectingNodeIds)
{
    nodesInsideCylinder(point, direction, 0.0f, intersectingNodeIds);
}

void Collision::nodesInsideCylinder(const QVector3D &point, const QVector3D &direction, float radius, std::vector<NodeId>& containedNodeIds)
{
    Plane plane(point, direction);

    const std::vector<NodeId>& nodeIds = _graphModel.graph().componentById(_componentId)->nodeIds();
    for(NodeId nodeId : nodeIds)
    {
        const QVector3D position = _graphModel.nodePositions().getScaledAndSmoothed(nodeId) + _offset;

        if(plane.sideForPoint(position) != Plane::Side::Front)
            continue;

        float distance = position.distanceToLine(point, direction);

        if(distance <= radius + _graphModel.nodeVisuals().at(nodeId)._size)
            containedNodeIds.push_back(nodeId);
    }
}

NodeId Collision::nearestNodeIntersectingLine(const QVector3D& point, const QVector3D& direction)
{
    return nearestNodeInsideCylinder(point, direction, 0.0f);
}

NodeId Collision::nearestNodeInsideCylinder(const QVector3D& point, const QVector3D& direction, float radius)
{
    std::vector<NodeId> nodeIds;

    nodesInsideCylinder(point, direction, radius, nodeIds);

    NodeId closestNodeId;
    float minimumDistance = std::numeric_limits<float>::max();

    for(NodeId nodeId : nodeIds)
    {
        float distance = _graphModel.nodePositions().getScaledAndSmoothed(nodeId).distanceToPoint(point);

        if(distance < minimumDistance)
        {
            minimumDistance = distance;
            closestNodeId = nodeId;
        }
    }

    return closestNodeId;
}
