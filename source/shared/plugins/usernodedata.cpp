#include "usernodedata.h"

#include "shared/graph/imutablegraph.h"
#include "shared/graph/igraphmodel.h"

void UserNodeData::initialise(IMutableGraph& mutableGraph)
{
    _indexes = std::make_unique<NodeArray<int>>(mutableGraph, -1);
}

void UserNodeData::addNodeId(NodeId nodeId)
{
    _indexes->set(nodeId, numValues());
    _rowToNodeIdMap[numValues()] = nodeId;
}

void UserNodeData::setNodeIdForRowIndex(NodeId nodeId, int row)
{
    _indexes->set(nodeId, row);
    _rowToNodeIdMap[row] = nodeId;
}

int UserNodeData::rowIndexForNodeId(NodeId nodeId) const
{
    int rowIndex = _indexes->get(nodeId);
    Q_ASSERT(rowIndex >= 0);
    return rowIndex;
}

NodeId UserNodeData::nodeIdForRowIndex(int row) const
{
    Q_ASSERT(u::contains(_rowToNodeIdMap, row));
    return _rowToNodeIdMap.at(row);
}

void UserNodeData::setValueByNodeId(NodeId nodeId, const QString& name, const QString& value)
{
    setValue(rowIndexForNodeId(nodeId), name, value);
}

QString UserNodeData::valueByNodeId(NodeId nodeId, const QString& name) const
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
         graphModel.setNodeName(nodeId, valueByNodeId(nodeId, firstUserDataVectorName()));
}

void UserNodeData::exposeToGraphModel(IGraphModel& graphModel)
{
    for(const auto& userDataVector : *this)
    {
        QString userDataVectorName = userDataVector.name();

        switch(userDataVector.type())
        {
        case UserDataVector::Type::Float:
            graphModel.dataField(userDataVectorName)
                    .setFloatValueFn([this, userDataVectorName](NodeId nodeId)
                    {
                        return valueByNodeId(nodeId, userDataVectorName).toFloat();
                    })
                    .setFloatMin(static_cast<float>(userDataVector.floatMin()))
                    .setFloatMax(static_cast<float>(userDataVector.floatMax()))
                    .setSearchable(true);
            break;

        case UserDataVector::Type::Integer:
            graphModel.dataField(userDataVectorName)
                    .setIntValueFn([this, userDataVectorName](NodeId nodeId)
                    {
                        return valueByNodeId(nodeId, userDataVectorName).toInt();
                    })
                    .setIntMin(userDataVector.intMin())
                    .setIntMax(userDataVector.intMax())
                    .setSearchable(true);
            break;

        case UserDataVector::Type::String:
            graphModel.dataField(userDataVectorName)
                    .setStringValueFn([this, userDataVectorName](NodeId nodeId)
                    {
                        return valueByNodeId(nodeId, userDataVectorName);
                    })
                    .setSearchable(true);
            break;

        default: break;
        }

        graphModel.dataField(userDataVectorName)
                .setDescription(QString(tr("%1 is a user defined attribute.")).arg(userDataVectorName));
    }
}
