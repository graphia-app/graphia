#ifndef IGRAPHMODEL_H
#define IGRAPHMODEL_H

#include "igraph.h"
#include "imutablegraph.h"
#include "../transform/idatafield.h"

class QString;

class IGraphModel
{
public:
    virtual ~IGraphModel() = default;

    virtual const IGraph& graph() const = 0;
    virtual IMutableGraph& mutableGraph() = 0;

    virtual QString nodeName(NodeId nodeId) const = 0;
    virtual void setNodeName(NodeId nodeId, const QString& name) = 0;

    virtual IDataField& dataField(const QString& name) = 0;
};

#endif // IGRAPHMODEL_H
