#ifndef NODEPOSITIONS_H
#define NODEPOSITIONS_H

#include "../graph/graph.h"
#include "../graph/grapharray.h"
#include "../utils/circularbuffer.h"
#include "../maths/boundingbox.h"
#include "../maths/boundingsphere.h"

#include <array>
#include <mutex>

#include <QVector3D>

static const int MAX_SMOOTHING = 8;

class MeanPosition : public CircularBuffer<QVector3D, MAX_SMOOTHING>
{
public:
    QVector3D mean(int samples) const;
};

class NodePositions : public NodeArray<MeanPosition>
{
private:
    std::recursive_mutex _mutex;
    bool _updated;
    float _scale;
    int _smoothing;

public:
    NodePositions(Graph& graph);

public:
    std::recursive_mutex& mutex() { return _mutex; }
    void setScale(float scale) { _scale = scale; }
    float scale() const { return _scale; }
    void setSmoothing(int smoothing) { _smoothing = smoothing; }
    int smoothing() const { return _smoothing; }

    const QVector3D& get(NodeId nodeId) const;
    const QVector3D getScaledAndSmoothed(NodeId nodeId) const;

    void update(const Graph& graph, std::function<QVector3D(NodeId, const QVector3D&)> f,
                float scale = 1.0f, int smoothing = 1);
    void executeIfUpdated(std::function<void()> f);

    static QVector3D centreOfMass(const NodePositions& nodePositions,
                                  const std::vector<NodeId>& nodeIds);
    static QVector3D centreOfMassScaledAndSmoothed(const NodePositions& nodePositions,
                                                   const std::vector<NodeId>& nodeIds);

    static std::vector<QVector3D> positionsVector(const NodePositions& nodePositions,
                                                  const std::vector<NodeId>& nodeIds);
    static std::vector<QVector3D> positionsVectorScaled(const NodePositions& nodePositions,
                                                        const std::vector<NodeId>& nodeIds);

    static BoundingSphere boundingSphere(const NodePositions& nodePositions,
                                         const std::vector<NodeId>& nodeIds);

private:
    using NodeArray<MeanPosition>::operator[];
    using NodeArray<MeanPosition>::at;
};


#endif // NODEPOSITIONS_H
