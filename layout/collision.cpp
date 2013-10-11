#include "collision.h"
#include "../maths/ray.h"
#include "../gl/graphscene.h"

NodeId Collision::closestNodeToLine(const QList<NodeId>& nodeIds, const QVector3D &point, const QVector3D &direction)
{
    NodeId closestNodeId = NullNodeId;
    float minimumDistance = std::numeric_limits<float>::max();

    for(NodeId nodeId : nodeIds)
    {
        const QVector3D& position = (*_nodePositions)[nodeId];
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
