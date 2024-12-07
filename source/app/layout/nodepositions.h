/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NODEPOSITIONS_H
#define NODEPOSITIONS_H

#include "shared/graph/grapharray.h"
#include "shared/utils/circularbuffer.h"
#include "app/maths/boundingsphere.h"
#include "app/maths/boundingbox.h"

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
    size_t _smoothing = 1;

    QVector3D getNoLocking(NodeId nodeId) const;

protected:
    const QVector3D& getNewestNoLocking(NodeId nodeId) const;

public:
    using NodeArray::NodeArray;

    void lock() const;
    void unlock() const;
    bool unlocked() const;

    void setScale(float scale) { _scale = scale; }
    float scale() const { return _scale; }

    void setSmoothing(size_t smoothing) { Q_ASSERT(smoothing <= MAX_SMOOTHING); _smoothing = smoothing; }
    size_t smoothing() const { return _smoothing; }

    QVector3D get(NodeId nodeId) const;
    std::vector<QVector3D> get(const std::vector<NodeId>& nodeIds) const;

    void flatten();

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
