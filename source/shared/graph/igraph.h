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

#ifndef IGRAPH_H
#define IGRAPH_H

#include "shared/graph/elementid.h"
#include "igrapharrayclient.h"

#include <vector>

class QString;

class INode
{
public:
    INode() = default;
    INode(const INode&) = default;
    INode(INode&&) noexcept = default;
    INode& operator=(const INode&) = default;
    INode& operator=(INode&&) noexcept = default;

    virtual ~INode() = default;

    virtual int degree() const = 0;
    virtual int inDegree() const = 0;
    virtual int outDegree() const = 0;
    virtual NodeId id() const = 0;

    virtual std::vector<EdgeId> inEdgeIds() const = 0;
    virtual std::vector<EdgeId> outEdgeIds() const = 0;
    virtual std::vector<EdgeId> edgeIds() const = 0;
};

class IEdge
{
public:
    IEdge() = default;
    IEdge(const IEdge&) = default;
    IEdge(IEdge&&) noexcept = default;
    IEdge& operator=(const IEdge&) = default;
    IEdge& operator=(IEdge&&) noexcept = default;

    virtual ~IEdge() = default;

    virtual NodeId sourceId() const = 0;
    virtual NodeId targetId() const = 0;
    virtual NodeId oppositeId(NodeId nodeId) const = 0;

    virtual bool isLoop() const = 0;

    virtual EdgeId id() const = 0;
};

class IGraphComponent;

class IGraph : public virtual IGraphArrayClient
{
public:
    ~IGraph() override = default;

    virtual const std::vector<NodeId>& nodeIds() const = 0;
    virtual size_t numNodes() const = 0;
    virtual const INode& nodeById(NodeId nodeId) const = 0;
    virtual bool containsNodeId(NodeId nodeId) const = 0;

    virtual const std::vector<EdgeId>& edgeIds() const = 0;
    virtual size_t numEdges() const = 0;
    virtual const IEdge& edgeById(EdgeId edgeId) const = 0;
    virtual bool containsEdgeId(EdgeId edgeId) const = 0;

    virtual const std::vector<ComponentId>& componentIds() const = 0;
    virtual size_t numComponents() const = 0;
    virtual const IGraphComponent* componentById(ComponentId componentId) const = 0;
    virtual bool containsComponentId(ComponentId componentId) const = 0;

    virtual std::vector<NodeId> sourcesOf(NodeId nodeId) const = 0;
    virtual std::vector<NodeId> targetsOf(NodeId nodeId) const = 0;
    virtual std::vector<NodeId> neighboursOf(NodeId nodeId) const = 0;

    virtual std::vector<EdgeId> edgeIdsBetween(NodeId nodeIdA, NodeId nodeIdB) const = 0;
    virtual EdgeId firstEdgeIdBetween(NodeId nodeIdA, NodeId nodeIdB) const = 0;
    virtual bool edgeExistsBetween(NodeId nodeIdA, NodeId nodeIdB) const = 0;

    virtual void setPhase(const QString& phase) const = 0;
    virtual void clearPhase() const = 0;
};

#endif // IGRAPH_H
