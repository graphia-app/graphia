/* Copyright © 2013-2021 Graphia Technologies Ltd.
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

#ifndef GRAPH_H
#define GRAPH_H

#include "shared/graph/igraph.h"
#include "shared/graph/elementid.h"
#include "shared/graph/elementid_containers.h"
#include "elementiddistinctsetcollection.h"
#include "graphconsistencychecker.h"

#include <QObject>

#include <vector>
#include <unordered_set>
#include <memory>
#include <mutex>
#include <algorithm>

class GraphComponent;
class ComponentManager;
class ComponentSplitSet;
class ComponentMergeSet;

class Node : public INode
{
    friend class MutableGraph;

private:
    NodeId _id;
    EdgeIdDistinctSet _inEdgeIds;
    EdgeIdDistinctSet _outEdgeIds;

public:
    int degree() const override { return _inEdgeIds.size() + _outEdgeIds.size(); }
    int inDegree() const override { return _inEdgeIds.size(); }
    int outDegree() const override { return _outEdgeIds.size(); }
    NodeId id() const override { return _id; }

    std::vector<EdgeId> inEdgeIds() const override;
    std::vector<EdgeId> outEdgeIds() const override;
    std::vector<EdgeId> edgeIds() const override;
};

class Edge : public IEdge
{
    friend class MutableGraph;

private:
    EdgeId _id;
    NodeId _sourceId;
    NodeId _targetId;

public:
    Edge() = default;

    explicit Edge(const IEdge& other) :
                  _id(other.id()),
                  _sourceId(other.sourceId()),
                  _targetId(other.targetId())
    {}

    explicit Edge(IEdge&& other) noexcept :
                  _id(other.id()),
                  _sourceId(other.sourceId()),
                  _targetId(other.targetId())
    {}

    Edge& operator=(const IEdge& other)
    {
        if(this != &other)
        {
            _id         = other.id();
            _sourceId   = other.sourceId();
            _targetId   = other.targetId();
        }

        return *this;
    }

    NodeId sourceId() const override { return _sourceId; }
    NodeId targetId() const override { return _targetId; }
    NodeId oppositeId(NodeId nodeId) const override
    {
        if(nodeId == _sourceId)
            return _targetId;

        if(nodeId == _targetId)
            return _sourceId;

        return {};
    }

    bool isLoop() const override { return _sourceId == _targetId; }

    EdgeId id() const override { return _id; }
};

class Graph : public QObject, public virtual IGraph
{
    Q_OBJECT

public:
    Graph();
    ~Graph() override;

    NodeId firstNodeId() const;
    bool containsNodeId(NodeId nodeId) const override;

    virtual MultiElementType typeOf(NodeId nodeId) const = 0;
    virtual ConstNodeIdDistinctSet mergedNodeIdsForNodeId(NodeId nodeId) const = 0;
    template<typename C> NodeIdSet mergedNodeIdsForNodeIds(const C& nodeIds) const
    {
        NodeIdSet mergedNodeIdSet;

        for(auto nodeId : nodeIds)
        {
            if(typeOf(nodeId) == MultiElementType::Tail)
                continue;

            const auto mergedNodeIds = mergedNodeIdsForNodeId(nodeId);
            mergedNodeIdSet.insert(mergedNodeIds.begin(), mergedNodeIds.end());
        }

        return mergedNodeIdSet;
    }
    virtual int multiplicityOf(NodeId nodeId) const = 0;

    EdgeId firstEdgeId() const;
    bool containsEdgeId(EdgeId edgeId) const override;

    virtual MultiElementType typeOf(EdgeId edgeId) const = 0;
    virtual ConstEdgeIdDistinctSet mergedEdgeIdsForEdgeId(EdgeId edgeId) const = 0;
    template<typename C> EdgeIdSet mergedEdgeIdsForEdgeIds(const C& edgeIds) const
    {
        EdgeIdSet mergedEdgeIdSet;

        for(auto edgeId : edgeIds)
        {
            if(typeOf(edgeId) == MultiElementType::Tail)
                continue;

            const auto mergedEdgeIds = mergedEdgeIdsForEdgeId(edgeId);
            mergedEdgeIdSet.insert(mergedEdgeIds.begin(), mergedEdgeIds.end());
        }

        return mergedEdgeIdSet;
    }
    virtual int multiplicityOf(EdgeId edgeId) const = 0;

    virtual EdgeIdDistinctSets edgeIdsForNodeId(NodeId nodeId) const = 0;

    template<typename C> EdgeIdSet edgeIdsForNodeIds(const C& nodeIds) const
    {
        EdgeIdSet edgeIds;

        for(auto nodeId : nodeIds)
        {
            for(auto edgeId : edgeIdsForNodeId(nodeId))
                edgeIds.insert(edgeId);
        }

        return edgeIds;
    }

    template<typename C> std::vector<Edge> edgesForNodeIds(const C& nodeIds) const
    {
        auto edgeIds = edgeIdsForNodeIds(nodeIds);
        std::vector<Edge> edges;
        edges.reserve(edgeIds.size());

        std::transform(edgeIds.begin(), edgeIds.end(), std::back_inserter(edges),
        [this](auto edgeId)
        {
            return Edge(edgeById(edgeId));
        });

        return edges;
    }

    virtual void reserve(const Graph& other);

    void enableComponentManagement();
    void disableComponentManagement();

    const std::vector<ComponentId>& componentIds() const override;
    int numComponents() const override;
    bool containsComponentId(ComponentId componentId) const override;
    const IGraphComponent* componentById(ComponentId componentId) const override;
    ComponentId componentIdOfNode(NodeId nodeId) const;
    ComponentId componentIdOfEdge(EdgeId edgeId) const;
    ComponentId componentIdOfLargestComponent() const;

    template<typename C> ComponentId componentIdOfLargestComponent(const C& componentIds) const
    {
        Q_ASSERT(componentManagementEnabled());

        if(!componentManagementEnabled())
            return {};

        ComponentId largestComponentId;
        int maxNumNodes = 0;
        for(auto componentId : componentIds)
        {
            auto component = componentById(componentId);
            if(component->numNodes() > maxNumNodes)
            {
                maxNumNodes = component->numNodes();
                largestComponentId = componentId;
            }
        }

        return largestComponentId;
    }

    bool componentManagementEnabled() const;

    std::vector<NodeId> sourcesOf(NodeId nodeId) const override;
    std::vector<NodeId> targetsOf(NodeId nodeId) const override;
    std::vector<NodeId> neighboursOf(NodeId nodeId) const override;

    // Call this to ensure the Graph is in a consistent state
    // Usually it is called automatically and is generally only
    // necessary when accessing the Graph before changes have
    // been completed
    // Note that this DOES NOT update components, so if this
    // information is required a ComponentManager instance
    // must be used directly
    virtual bool update() { return false; }

    // Informational messages to indicate progress
    void setPhase(const QString& phase) const override;
    void clearPhase() const override;
    virtual QString phase() const;

    void dumpToQDebug(int detail) const;

private:
    template<typename, typename> friend class NodeArray;
    template<typename, typename> friend class EdgeArray;
    template<typename, typename> friend class ComponentArray;

    NodeId _nextNodeId;
    EdgeId _nextEdgeId;

    mutable std::mutex _nodeArraysMutex;
    mutable std::unordered_set<IGraphArray*> _nodeArrays;
    mutable std::mutex _edgeArraysMutex;
    mutable std::unordered_set<IGraphArray*> _edgeArrays;

    std::unique_ptr<ComponentManager> _componentManager;

    mutable std::recursive_mutex _phaseMutex;
    mutable QString _phase;
    GraphConsistencyChecker _graphConsistencyChecker;

    void insertNodeArray(IGraphArray* nodeArray) const override;
    void eraseNodeArray(IGraphArray* nodeArray) const override;

    void insertEdgeArray(IGraphArray* edgeArray) const override;
    void eraseEdgeArray(IGraphArray* edgeArray) const override;

    int numComponentArrays() const override;
    void insertComponentArray(IGraphArray* componentArray) const override;
    void eraseComponentArray(IGraphArray* componentArray) const override;

    bool isComponentManaged() const override { return _componentManager != nullptr; }

protected:
    NodeId nextNodeId() const override;
    NodeId lastNodeIdInUse() const override;
    NodeId largestNodeId() const { return nextNodeId() - 1; }
    virtual void reserveNodeId(NodeId nodeId);
    EdgeId nextEdgeId() const override;
    EdgeId lastEdgeIdInUse() const override;
    EdgeId largestEdgeId() const { return nextEdgeId() - 1; }
    virtual void reserveEdgeId(EdgeId edgeId);

    void clear();

signals:
    // The signals are listed here in the order in which they are emitted
    void graphWillChange(const Graph*);

    void nodeAdded(const Graph*, NodeId);
    void nodeRemoved(const Graph*, NodeId);
    void edgeAdded(const Graph*, EdgeId);
    void edgeRemoved(const Graph*, EdgeId);

    void componentsWillMerge(const Graph*, const ComponentMergeSet&);
    void componentWillBeRemoved(const Graph*, ComponentId, bool);
    void componentAdded(const Graph*, ComponentId, bool);
    void componentSplit(const Graph*, const ComponentSplitSet&);

    void nodeRemovedFromComponent(const Graph*, NodeId, ComponentId);
    void edgeRemovedFromComponent(const Graph*, EdgeId, ComponentId);
    void nodeAddedToComponent(const Graph*, NodeId, ComponentId);
    void edgeAddedToComponent(const Graph*, EdgeId, ComponentId);

    void graphChanged(const Graph*, bool changeOccurred);

    void phaseChanged() const; // clazy:exclude=const-signal-or-slot
};

#endif // GRAPH_H
