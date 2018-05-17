#ifndef IGRAPHMODEL_H
#define IGRAPHMODEL_H

#include "shared/graph/elementid.h"
#include "shared/graph/elementtype.h"

#include <vector>
#include <QString>

class IGraph;
class IMutableGraph;
class IAttribute;
struct IElementVisual;

class IGraphModel
{
public:
    virtual ~IGraphModel() = default;

protected:
    virtual IMutableGraph& mutableGraphImpl() = 0;
    virtual const IGraph& graphImpl() const = 0;

    virtual const IElementVisual& nodeVisualImpl(NodeId nodeId) const = 0;
    virtual const IElementVisual& edgeVisualImpl(EdgeId edgeId) const = 0;

public:
    IMutableGraph& mutableGraph() { return mutableGraphImpl(); }
    const IGraph& graph() const { return graphImpl(); }

    const IElementVisual& nodeVisual(NodeId nodeId) const { return nodeVisualImpl(nodeId); }
    const IElementVisual& edgeVisual(EdgeId edgeId) const { return edgeVisualImpl(edgeId); }

    virtual QString nodeName(NodeId nodeId) const = 0;
    virtual void setNodeName(NodeId nodeId, const QString& name) = 0;

    virtual IAttribute& createAttribute(QString name) = 0;
    virtual const IAttribute* attributeByName(const QString& name) const = 0;
    virtual bool attributeExists(const QString& name) const = 0;
    virtual std::vector<QString> attributeNames(ElementType elementType = ElementType::All) const = 0;
    virtual std::vector<NodeId> nodeIdsByAttributeValue(const QString& attributeName, const QString& attributeValue) const = 0;
};

#endif // IGRAPHMODEL_H
