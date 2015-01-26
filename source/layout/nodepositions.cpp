#include "nodepositions.h"

QVector3D MeanPosition::mean(int samples) const
{
    samples = std::min(samples, static_cast<int>(size()));

    QVector3D result;
    float reciprocal = 1.0f / samples;

    for(int i = 0; i > -samples; i--)
        result += at(i) * reciprocal;

    return result;
}

NodePositions::NodePositions(Graph& graph) :
    NodeArray<MeanPosition>(graph),
    _updated(false),
    _scale(1.0f),
    _smoothing(1)
{}

const QVector3D& NodePositions::get(NodeId nodeId) const
{
    return _array[nodeId].front();
}

const QVector3D NodePositions::getScaledAndSmoothed(NodeId nodeId) const
{
    return _array[nodeId].mean(_smoothing) * _scale;
}

void NodePositions::update(const ReadOnlyGraph& graph, std::function<QVector3D(NodeId, const QVector3D&)> f,
                           float scale, int smoothing)
{
    Q_ASSERT(smoothing <= MAX_SMOOTHING);
    std::unique_lock<std::recursive_mutex> lock(_mutex);

    setScale(scale);
    setSmoothing(smoothing);

    for(NodeId nodeId : graph.nodeIds())
        _array.at(nodeId).push_back(f(nodeId, _array.at(nodeId).front()));

    _updated = true;
}

void NodePositions::executeIfUpdated(std::function<void()> f)
{
    std::unique_lock<std::recursive_mutex> lock(_mutex);
    if(_updated)
    {
        f();
        _updated = false;
    }
}

QVector3D NodePositions::centreOfMass(const NodePositions& nodePositions,
                                      const std::vector<NodeId>& nodeIds)
{
    float reciprocal = 1.0f / nodeIds.size();
    QVector3D centreOfMass = QVector3D();

    for(auto nodeId : nodeIds)
        centreOfMass += (nodePositions.get(nodeId) * reciprocal);

    return centreOfMass;
}

QVector3D NodePositions::centreOfMassScaled(const NodePositions& nodePositions, const std::vector<NodeId>& nodeIds)
{
    return centreOfMass(nodePositions, nodeIds) * nodePositions.scale();
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
