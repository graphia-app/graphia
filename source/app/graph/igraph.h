#ifndef IGRAPH_H
#define IGRAPH_H

#include "elementid.h"

class IGraphArray;

class INode
{
public:
    virtual ~INode() = default;

    virtual int degree() const = 0;
    virtual NodeId id() const = 0;
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

class IGraph
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

    // GraphArray support
    virtual NodeId nextNodeId() const = 0;
    virtual EdgeId nextEdgeId() const = 0;

    virtual void insertNodeArray(IGraphArray* nodeArray) const = 0;
    virtual void eraseNodeArray(IGraphArray* nodeArray) const = 0;

    virtual void insertEdgeArray(IGraphArray* edgeArray) const = 0;
    virtual void eraseEdgeArray(IGraphArray* edgeArray) const = 0;

    virtual int numComponentArrays() const = 0;
    virtual void insertComponentArray(IGraphArray* componentArray) const = 0;
    virtual void eraseComponentArray(IGraphArray* componentArray) const = 0;

    virtual bool isComponentManaged() const = 0;
};

#endif // IGRAPH_H
