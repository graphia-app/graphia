#ifndef NODEATTRIBUTES_H
#define NODEATTRIBUTES_H

#include "attributes.h"

#include <memory>
#include "shared/graph/grapharray.h"

class IMutableGraph;
class IGraphModel;

class NodeAttributes : public Attributes
{
    Q_OBJECT

private:
    std::unique_ptr<NodeArray<int>> _indexes;

public:
    void initialise(IMutableGraph& mutableGraph);

    void addNodeId(NodeId nodeId);
    void setNodeIdForRowIndex(NodeId nodeId, int row);
    int rowIndexForNodeId(NodeId nodeId) const;

    void setValueByNodeId(NodeId nodeId, const QString& name, const QString& value);
    QString valueByNodeId(NodeId nodeId, const QString& name) const;

    void setNodeNamesToFirstAttribute(IGraphModel& graphModel);
    void exposeToGraphModel(IGraphModel& graphModel);
};

#endif // NODEATTRIBUTES_H
