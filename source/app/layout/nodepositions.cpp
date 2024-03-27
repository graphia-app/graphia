/* Copyright © 2013-2023 Graphia Technologies Ltd.
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

#include "nodepositions.h"

#include <cmath>
#include <numeric>

QVector3D NodePositions::getNoLocking(NodeId nodeId) const
{
    return elementFor(nodeId).mean(_smoothing) * _scale;
}

const QVector3D& NodePositions::getNewestNoLocking(NodeId nodeId) const
{
    return elementFor(nodeId).newest();
}

void NodePositions::lock() const
{
    _mutex.lock();
    _threadId = std::this_thread::get_id();
}

void NodePositions::unlock() const
{
    _threadId = {};
    _mutex.unlock();
}

bool NodePositions::unlocked() const
{
    return _threadId == std::thread::id{};
}

QVector3D NodePositions::get(NodeId nodeId) const
{
    const std::unique_lock<const NodePositions> lock(*this);

    return getNoLocking(nodeId);
}

std::vector<QVector3D> NodePositions::get(const std::vector<NodeId>& nodeIds) const
{
    const std::unique_lock<const NodePositions> lock(*this);

    std::vector<QVector3D> positions;
    positions.reserve(nodeIds.size());

    for(auto nodeId : nodeIds)
        positions.emplace_back(getNoLocking(nodeId));

    return positions;
}

void NodePositions::flatten()
{
    const std::unique_lock<std::recursive_mutex> lock(_mutex);

    generate([this](NodeId nodeId)
    {
        auto positions = elementFor(nodeId);

        for(size_t i = 0; i < positions.size(); i++)
            positions.at(i).setZ(0.0f);

        return positions;
    });
}

void NodePositions::update(const NodePositions& other)
{
    const std::unique_lock<const NodePositions> lock(*this);

    _array = other._array;
}

template<typename GetFn>
static QVector3D centreOfMassWithFn(const std::vector<NodeId>& nodeIds, const GetFn& getFn)
{
    float reciprocal = 1.0f / static_cast<float>(nodeIds.size());

    return std::accumulate(nodeIds.begin(), nodeIds.end(), QVector3D(),
    [reciprocal, &getFn](const auto& com, auto nodeId)
    {
        return com + (getFn(nodeId) * reciprocal);
    });
}

QVector3D NodePositions::centreOfMass(const std::vector<NodeId>& nodeIds) const
{
    const std::unique_lock<const NodePositions> lock(*this);

    return centreOfMassWithFn(nodeIds, [this](NodeId nodeId) { return getNoLocking(nodeId); });
}

const QVector3D& NodePositions::at(NodeId nodeId) const
{
    const std::unique_lock<const NodePositions> lock(*this);

    return getNewestNoLocking(nodeId);
}

const QVector3D& NodeLayoutPositions::get(NodeId nodeId) const
{
    Q_ASSERT(unlocked());

    return getNewestNoLocking(nodeId);
}

void NodeLayoutPositions::set(NodeId nodeId, const QVector3D& position)
{
    Q_ASSERT(unlocked());
    Q_ASSERT(!std::isnan(position.x()) && !std::isnan(position.y()) && !std::isnan(position.z()));

    elementFor(nodeId).push_back(position);
}

void NodeLayoutPositions::set(const std::vector<NodeId>& nodeIds, const ExactNodePositions& nodePositions)
{
    Q_ASSERT(unlocked());

    for(auto nodeId : nodeIds)
    {
        auto position = nodePositions.at(nodeId);

        Q_ASSERT(!std::isnan(position.x()) && !std::isnan(position.y()) && !std::isnan(position.z()));
        elementFor(nodeId).fill(position);
    }
}

QVector3D NodeLayoutPositions::centreOfMass(const std::vector<NodeId>& nodeIds) const
{
    Q_ASSERT(unlocked());

    return centreOfMassWithFn(nodeIds, [this](NodeId nodeId) { return getNewestNoLocking(nodeId); });
}

BoundingBox3D NodeLayoutPositions::boundingBox(const std::vector<NodeId>& nodeIds) const
{
    Q_ASSERT(unlocked());

    if(nodeIds.empty())
        return {};

    Q_ASSERT(!nodeIds.empty());

    auto firstPosition = getNewestNoLocking(nodeIds.front());
    BoundingBox3D boundingBox(firstPosition, firstPosition);

    for(const NodeId nodeId : nodeIds)
        boundingBox.expandToInclude(get(nodeId));

    return boundingBox;
}
