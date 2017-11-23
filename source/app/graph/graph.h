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
    Edge() {}

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

        return NodeId();
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
            const auto mergedNodeIds = mergedNodeIdsForNodeId(nodeId);
            mergedNodeIdSet.insert(mergedNodeIds.begin(), mergedNodeIds.end());
        }

        return mergedNodeIdSet;
    }

    EdgeId firstEdgeId() const;
    bool containsEdgeId(EdgeId edgeId) const override;

    virtual MultiElementType typeOf(EdgeId edgeId) const = 0;
    virtual ConstEdgeIdDistinctSet mergedEdgeIdsForEdgeId(EdgeId edgeId) const = 0;
    template<typename C> EdgeIdSet mergedEdgeIdsForEdgeIds(const C& edgeIds) const
    {
        EdgeIdSet mergedEdgeIdSet;

        for(auto edgeId : edgeIds)
        {
            const auto mergedEdgeIds = mergedEdgeIdsForEdgeId(edgeId);
            mergedEdgeIdSet.insert(mergedEdgeIds.begin(), mergedEdgeIds.end());
        }

        return mergedEdgeIdSet;
    }

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

        for(auto edgeId : edgeIds)
            edges.emplace_back(edgeById(edgeId));

        return edges;
    }

    virtual void reserve(const Graph& other);

    void enableComponentManagement();

    const std::vector<ComponentId>& componentIds() const override;
    int numComponents() const override;
    bool containsComponentId(ComponentId componentId) const override;
    const IGraphComponent* componentById(ComponentId componentId) const override;
    ComponentId componentIdOfNode(NodeId nodeId) const;
    ComponentId componentIdOfEdge(EdgeId edgeId) const;
    ComponentId componentIdOfLargestComponent() const;

    template<typename C> ComponentId componentIdOfLargestComponent(const C& componentIds) const
    {
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
    virtual void update() {}

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
    mutable QString _subPhase;
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
    NodeId largestNodeId() const { return nextNodeId() - 1; }
    virtual void reserveNodeId(NodeId nodeId);
    EdgeId nextEdgeId() const override;
    EdgeId largestEdgeId() const { return nextEdgeId() - 1; }
    virtual void reserveEdgeId(EdgeId edgeId);

    void clear();

signals:
    // The signals are listed here in the order in which they are emitted
    void graphWillChange(const Graph*) const;

    void nodeAdded(const Graph*, NodeId) const;
    void nodeRemoved(const Graph*, NodeId) const;
    void edgeAdded(const Graph*, EdgeId) const;
    void edgeRemoved(const Graph*, EdgeId) const;

    void componentsWillMerge(const Graph*, const ComponentMergeSet&) const;
    void componentWillBeRemoved(const Graph*, ComponentId, bool) const;
    void componentAdded(const Graph*, ComponentId, bool) const;
    void componentSplit(const Graph*, const ComponentSplitSet&) const;

    void nodeRemovedFromComponent(const Graph*, NodeId, ComponentId) const;
    void edgeRemovedFromComponent(const Graph*, EdgeId, ComponentId) const;
    void nodeAddedToComponent(const Graph*, NodeId, ComponentId) const;
    void edgeAddedToComponent(const Graph*, EdgeId, ComponentId) const;

    void graphChanged(const Graph*, bool changeOccurred) const;

    void phaseChanged() const;
};

#endif // GRAPH_H
