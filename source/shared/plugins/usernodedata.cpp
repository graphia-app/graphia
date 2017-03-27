#include "usernodedata.h"

#include "shared/graph/imutablegraph.h"
#include "shared/graph/igraphmodel.h"

void UserNodeData::initialise(IMutableGraph& mutableGraph)
{
    _indexes = std::make_unique<NodeArray<size_t>>(mutableGraph, 0);
}

void UserNodeData::addNodeId(NodeId nodeId)
{
    _indexes->set(nodeId, numValues());
    _rowToNodeIdMap[numValues()] = nodeId;
}

void UserNodeData::setNodeIdForRowIndex(NodeId nodeId, size_t row)
{
    _indexes->set(nodeId, row);
    _rowToNodeIdMap[row] = nodeId;
}

size_t UserNodeData::rowIndexForNodeId(NodeId nodeId) const
{
    return _indexes->get(nodeId);
}

NodeId UserNodeData::nodeIdForRowIndex(size_t row) const
{
    Q_ASSERT(u::contains(_rowToNodeIdMap, row));
    return _rowToNodeIdMap.at(row);
}

void UserNodeData::setValueByNodeId(NodeId nodeId, const QString& name, const QString& value)
{
    setValue(rowIndexForNodeId(nodeId), name, value);
}

QVariant UserNodeData::valueByNodeId(NodeId nodeId, const QString& name) const
{
    return value(rowIndexForNodeId(nodeId), name);
}

void UserNodeData::setNodeNamesToFirstUserDataVector(IGraphModel& graphModel)
{
    if(empty())
        return;

    // We must use the mutable version of the graph here as the transformed one
    // probably won't contain all of the node ids
    for(NodeId nodeId : graphModel.mutableGraph().nodeIds())
         graphModel.setNodeName(nodeId, valueByNodeId(nodeId, firstUserDataVectorName()).toString());
}

void UserNodeData::exposeAsAttributes(IGraphModel& graphModel)
{
    for(const auto& userDataVector : *this)
    {
        const auto& userDataVectorName = userDataVector.name();

        switch(userDataVector.type())
        {
        case UserDataVector::Type::Float:
            graphModel.attribute(userDataVectorName)
                    .setFloatValueFn([this, userDataVectorName](NodeId nodeId)
                    {
                        return valueByNodeId(nodeId, userDataVectorName).toFloat();
                    })
                    .setFlag(AttributeFlag::AutoRangeMutable)
                    .setSearchable(true);
            break;

        case UserDataVector::Type::Int:
            graphModel.attribute(userDataVectorName)
                    .setIntValueFn([this, userDataVectorName](NodeId nodeId)
                    {
                        return valueByNodeId(nodeId, userDataVectorName).toInt();
                    })
                    .setFlag(AttributeFlag::AutoRangeMutable)
                    .setSearchable(true);
            break;

        case UserDataVector::Type::String:
            graphModel.attribute(userDataVectorName)
                    .setStringValueFn([this, userDataVectorName](NodeId nodeId)
                    {
                        return valueByNodeId(nodeId, userDataVectorName).toString();
                    })
                    .setSearchable(true);
            break;

        default: break;
        }

        graphModel.attribute(userDataVectorName)
                .setDescription(QString(tr("%1 is a user defined attribute.")).arg(userDataVectorName));
    }
}
