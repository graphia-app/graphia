#ifndef IGRAPH_H
#define IGRAPH_H

#include "shared/graph/elementid.h"
#include "igrapharrayclient.h"

#include <vector>

class QString;

class INode
{
public:
    virtual ~INode() = default;

    virtual int degree() const = 0;
    virtual NodeId id() const = 0;

    virtual std::vector<EdgeId> inEdgeIds() const = 0;
    virtual std::vector<EdgeId> outEdgeIds() const = 0;
    virtual std::vector<EdgeId> edgeIds() const = 0;
};

class IEdge
{
public:
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
    virtual ~IGraph() = default;

    virtual const std::vector<NodeId>& nodeIds() const = 0;
    virtual int numNodes() const = 0;
    virtual const INode& nodeById(NodeId nodeId) const = 0;
    virtual bool containsNodeId(NodeId nodeId) const = 0;

    virtual const std::vector<EdgeId>& edgeIds() const = 0;
    virtual int numEdges() const = 0;
    virtual const IEdge& edgeById(EdgeId edgeId) const = 0;
    virtual bool containsEdgeId(EdgeId edgeId) const = 0;

    virtual const std::vector<ComponentId>& componentIds() const = 0;
    virtual int numComponents() const = 0;
    virtual const IGraphComponent* componentById(ComponentId componentId) const = 0;
    virtual bool containsComponentId(ComponentId componentId) const = 0;

    virtual std::vector<NodeId> sourcesOf(NodeId nodeId) const = 0;
    virtual std::vector<NodeId> targetsOf(NodeId nodeId) const = 0;
    virtual std::vector<NodeId> neighboursOf(NodeId nodeId) const = 0;

    virtual void setPhase(const QString& phase) const = 0;
    virtual void clearPhase() const = 0;
};

#endif // IGRAPH_H
