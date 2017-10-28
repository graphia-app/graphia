#ifndef IGRAPHMODEL_H
#define IGRAPHMODEL_H

#include "shared/graph/elementid.h"
#include "shared/graph/elementtype.h"

#include <vector>
#include <QString>

class IGraph;
class IMutableGraph;
class IAttribute;
class IElementVisual;

class IGraphModel
{
public:
    virtual ~IGraphModel() = default;

    virtual const IGraph& graph() const = 0;
    virtual IMutableGraph& mutableGraph() = 0;

    virtual const IElementVisual& nodeVisual(NodeId nodeId) const = 0;
    virtual const IElementVisual& edgeVisual(EdgeId edgeId) const = 0;

    virtual QString nodeName(NodeId nodeId) const = 0;
    virtual void setNodeName(NodeId nodeId, const QString& name) = 0;

    virtual IAttribute& createAttribute(QString name) = 0;
    virtual const IAttribute* attributeByName(const QString& name) const = 0;
    virtual std::vector<QString> attributeNames(ElementType elementType = ElementType::All) const = 0;
};

#endif // IGRAPHMODEL_H
