#include "nodeattributes.h"

#include "shared/graph/imutablegraph.h"
#include "shared/graph/igraphmodel.h"

void NodeAttributes::initialise(IMutableGraph& mutableGraph)
{
    _indexes = std::make_unique<NodeArray<int>>(mutableGraph, -1);
}

void NodeAttributes::addNodeId(NodeId nodeId)
{
    _indexes->set(nodeId, size());
}

void NodeAttributes::setNodeIdForRowIndex(NodeId nodeId, int row)
{
    _indexes->set(nodeId, row);
}

int NodeAttributes::rowIndexForNodeId(NodeId nodeId) const
{
    int rowIndex = _indexes->get(nodeId);
    Q_ASSERT(rowIndex >= 0);
    return rowIndex;
}

void NodeAttributes::setValueByNodeId(NodeId nodeId, const QString& name, const QString& value)
{
    setValue(rowIndexForNodeId(nodeId), name, value);
}

QString NodeAttributes::valueByNodeId(NodeId nodeId, const QString& name) const
{
    return value(rowIndexForNodeId(nodeId), name);
}

void NodeAttributes::exposeToGraphModel(IGraphModel& graphModel)
{
    for(auto& nodeAttribute : *this)
    {
        QString nodeAttributeName = nodeAttribute.name();

        switch(nodeAttribute.type())
        {
        case Attribute::Type::Float:
            graphModel.dataField(nodeAttribute.name())
                    .setFloatValueFn([this, nodeAttributeName](NodeId nodeId)
                    {
                        return valueByNodeId(nodeId, nodeAttributeName).toFloat();
                    })
                    .setFloatMin(static_cast<float>(nodeAttribute.floatMin()))
                    .setFloatMax(static_cast<float>(nodeAttribute.floatMax()))
                    .setSearchable(true);
            break;

        case Attribute::Type::Integer:
            graphModel.dataField(nodeAttribute.name())
                    .setIntValueFn([this, nodeAttributeName](NodeId nodeId)
                    {
                        return valueByNodeId(nodeId, nodeAttributeName).toInt();
                    })
                    .setIntMin(nodeAttribute.intMin())
                    .setIntMax(nodeAttribute.intMax())
                    .setSearchable(true);
            break;

        case Attribute::Type::String:
            graphModel.dataField(nodeAttribute.name())
                    .setStringValueFn([this, nodeAttributeName](NodeId nodeId)
                    {
                        return valueByNodeId(nodeId, nodeAttributeName);
                    })
                    .setSearchable(true);
            break;

        default: break;
        }
    }
}
