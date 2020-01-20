#ifndef NODEPOSITIONS_H
#define NODEPOSITIONS_H

#include "shared/graph/grapharray.h"
#include "shared/utils/circularbuffer.h"
#include "maths/boundingsphere.h"
#include "maths/boundingbox.h"

#include <array>
#include <mutex>
#include <thread>

#include <QVector3D>

static const int MAX_SMOOTHING = 8;

class MeanPosition : public CircularBuffer<QVector3D, MAX_SMOOTHING>
{ public: MeanPosition() { push_back({}); } };

class NodePositions : public NodeArray<MeanPosition>
{
private:
    mutable std::recursive_mutex _mutex;
    mutable std::thread::id _threadId;

    float _scale = 1.0f;
    int _smoothing = 1;

protected:
    const QVector3D& getUnsafe(NodeId nodeId) const;

public:
    using NodeArray::NodeArray;

    void lock() const;
    void unlock() const;
    bool unlocked() const;

    void setScale(float scale) { _scale = scale; }
    float scale() const { return _scale; }

    void setSmoothing(int smoothing) { Q_ASSERT(smoothing <= MAX_SMOOTHING); _smoothing = smoothing; }
    int smoothing() const { return _smoothing; }

    QVector3D get(NodeId nodeId) const;

    void update(const NodePositions& other);

    QVector3D centreOfMass(const std::vector<NodeId>& nodeIds) const;

    // This is only here as NativeSaver requires its interface
    const QVector3D& at(NodeId nodeId) const;

    // Delete base accessors that could be harmful
    MeanPosition& operator[](NodeId nodeId) = delete;
    const MeanPosition& operator[](NodeId nodeId) const = delete;
    MeanPosition& at(NodeId nodeId) = delete;
};

using ExactNodePositions = NodeArray<QVector3D>;

// This interface is exposed to the Layout algorithms only, giving
// them a fast interface to getting and setting node positions,
// without needing to lock
class NodeLayoutPositions : public NodePositions
{
public:
    using NodePositions::NodePositions;
    using NodePositions::set;

    // These accessors get and set the raw node positions, i.e. before
    // they are scaled and/or smoothed
    const QVector3D& get(NodeId nodeId) const;
    void set(NodeId nodeId, const QVector3D& position);
    void set(const std::vector<NodeId>& nodeIds, const ExactNodePositions& nodePositions);

    QVector3D centreOfMass(const std::vector<NodeId>& nodeIds) const;
    BoundingBox3D boundingBox(const std::vector<NodeId>& nodeIds) const;
};

// Ensure data members aren't added to NodeLayoutPositions, so that
// NodePositions::update doesn't slice
static_assert(sizeof(NodePositions) == sizeof(NodeLayoutPositions));

#endif // NODEPOSITIONS_H
