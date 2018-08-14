#ifndef IGRAPHARRAYCLIENT_H
#define IGRAPHARRAYCLIENT_H

#include "shared/graph/elementid.h"

class IGraphArray;

// Graph classes must implement this interface for a GraphArray
// to be able to associate itself with said graph
class IGraphArrayClient
{
public:
    virtual ~IGraphArrayClient() = default;

    virtual NodeId nextNodeId() const = 0;
    virtual EdgeId nextEdgeId() const = 0;

    virtual NodeId lastNodeIdInUse() const = 0;
    virtual EdgeId lastEdgeIdInUse() const = 0;

    virtual void insertNodeArray(IGraphArray* nodeArray) const = 0;
    virtual void eraseNodeArray(IGraphArray* nodeArray) const = 0;

    virtual void insertEdgeArray(IGraphArray* edgeArray) const = 0;
    virtual void eraseEdgeArray(IGraphArray* edgeArray) const = 0;

    virtual int numComponentArrays() const = 0;
    virtual void insertComponentArray(IGraphArray* componentArray) const = 0;
    virtual void eraseComponentArray(IGraphArray* componentArray) const = 0;

    virtual bool isComponentManaged() const = 0;
};

#endif // IGRAPHARRAYCLIENT_H
