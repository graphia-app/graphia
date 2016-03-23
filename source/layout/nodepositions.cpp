#include "nodepositions.h"

NodePositions::NodePositions(const Graph& graph) :
    NodeArray<MeanPosition>(graph)
{}

const QVector3D& NodePositions::get(NodeId nodeId) const
{
    return _array[nodeId].newest();
}

const QVector3D NodePositions::getScaledAndSmoothed(NodeId nodeId) const
{
    return _array[nodeId].mean(_smoothing) * _scale;
}

void NodePositions::set(NodeId nodeId, const QVector3D& position)
{
    _array.at(nodeId).push_back(position);
}

void NodePositions::update(const NodePositions& other)
{
    std::unique_lock<std::recursive_mutex> lock(_mutex);

    _array = other._array;
    _updated = true;
}

QVector3D NodePositions::centreOfMass(const NodePositions& nodePositions,
                                      const std::vector<NodeId>& nodeIds)
{
    float reciprocal = 1.0f / nodeIds.size();
    QVector3D centreOfMass;

    for(auto nodeId : nodeIds)
        centreOfMass += (nodePositions.get(nodeId) * reciprocal);

    return centreOfMass;
}

QVector3D NodePositions::centreOfMassScaledAndSmoothed(const NodePositions& nodePositions, const std::vector<NodeId>& nodeIds)
{
    float reciprocal = 1.0f / nodeIds.size();
    QVector3D centreOfMass = QVector3D();

    for(auto nodeId : nodeIds)
        centreOfMass += (nodePositions.getScaledAndSmoothed(nodeId) * reciprocal);

    return centreOfMass;
}

std::vector<QVector3D> NodePositions::positionsVector(const NodePositions& nodePositions, const std::vector<NodeId>& nodeIds)
{
    std::vector<QVector3D> positionsVector;
    for(NodeId nodeId : nodeIds)
        positionsVector.push_back(nodePositions.get(nodeId));

    return positionsVector;
}

std::vector<QVector3D> NodePositions::positionsVectorScaled(const NodePositions& nodePositions, const std::vector<NodeId>& nodeIds)
{
    std::vector<QVector3D> positionsVector;
    for(NodeId nodeId : nodeIds)
        positionsVector.push_back(nodePositions.getScaledAndSmoothed(nodeId));

    return positionsVector;
}

// http://stackoverflow.com/a/24818473
BoundingSphere NodePositions::boundingSphere(const NodePositions& nodePositions, const std::vector<NodeId>& nodeIds)
{
    QVector3D center = nodePositions.getScaledAndSmoothed(nodeIds.front());
    float radius = 0.0001f;
    QVector3D pos, diff;
    float len, alpha, alphaSq;

    for(int i = 0; i < 2; i++)
    {
        for(auto& nodeId : nodeIds)
        {
            pos = nodePositions.getScaledAndSmoothed(nodeId);
            diff = pos - center;
            len = diff.length();

            if(len > radius)
            {
                alpha = len / radius;
                alphaSq = alpha * alpha;
                radius = 0.5f * (alpha + 1.0f / alpha) * radius;
                center = 0.5f * ((1.0f + 1.0f / alphaSq) * center + (1.0f - 1.0f / alphaSq) * pos);
            }
        }
    }

    for(auto& nodeId : nodeIds)
    {
        pos = nodePositions.getScaledAndSmoothed(nodeId);
        diff = pos - center;
        len = diff.length();
        if(len > radius)
        {
            radius = (radius + len) / 2.0f;
            center = center + ((len - radius) / len * diff);
        }
    }

    return BoundingSphere(center, radius);
}
