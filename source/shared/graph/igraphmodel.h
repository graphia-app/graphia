#ifndef IGRAPHMODEL_H
#define IGRAPHMODEL_H

#include "igraph.h"
#include "imutablegraph.h"
#include "shared/attributes/iattribute.h"
#include "shared/ui/visualisations/elementvisual.h"

class QString;

class IGraphModel
{
public:
    virtual ~IGraphModel() = default;

    virtual const IGraph& graph() const = 0;
    virtual IMutableGraph& mutableGraph() = 0;

    virtual const ElementVisual& nodeVisual(NodeId nodeId) const = 0;
    virtual const ElementVisual& edgeVisual(EdgeId edgeId) const = 0;

    virtual QString nodeName(NodeId nodeId) const = 0;
    virtual void setNodeName(NodeId nodeId, const QString& name) = 0;

    virtual IAttribute& createAttribute(const QString& name) = 0;
    virtual const IAttribute* attributeByName(const QString& name) const = 0;
    virtual std::vector<QString> attributeNames(ElementType elementType = ElementType::All) const = 0;
};

#endif // IGRAPHMODEL_H
