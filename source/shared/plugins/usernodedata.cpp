#include "usernodedata.h"

#include "shared/graph/imutablegraph.h"
#include "shared/graph/igraphmodel.h"

#include <QJsonArray>

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
        auto& attribute = graphModel.createAttribute(userDataVectorName)
                .setSearchable(true)
                .setUserDefined(true);

        switch(userDataVector.type())
        {
        case UserDataVector::Type::Float:
            attribute.setFloatValueFn([this, userDataVectorName](NodeId nodeId)
                    {
                        return valueByNodeId(nodeId, userDataVectorName).toFloat();
                    })
                    .setFlag(AttributeFlag::AutoRangeMutable);
            break;

        case UserDataVector::Type::Int:
            attribute.setIntValueFn([this, userDataVectorName](NodeId nodeId)
                    {
                        return valueByNodeId(nodeId, userDataVectorName).toInt();
                    })
                    .setFlag(AttributeFlag::AutoRangeMutable);
            break;

        case UserDataVector::Type::String:
            attribute.setStringValueFn([this, userDataVectorName](NodeId nodeId)
                    {
                        return valueByNodeId(nodeId, userDataVectorName).toString();
                    });
            break;

        default: break;
        }

        bool hasMissingValues = std::any_of(userDataVector.begin(), userDataVector.end(),
                                            [](const auto& v) { return v.isEmpty(); });

        if(hasMissingValues)
        {
            attribute.setValueMissingFn([this, userDataVectorName](NodeId nodeId)
            {
               return valueByNodeId(nodeId, userDataVectorName).toString().isEmpty();
            });
        }

        attribute.setDescription(QString(tr("%1 is a user defined attribute.")).arg(userDataVectorName));
    }
}

QJsonObject UserNodeData::save(const IMutableGraph& graph, const ProgressFn& progressFn) const
{
    QJsonObject jsonObject = UserData::save(progressFn);

    QJsonArray jsonArray;
    for(auto nodeId : graph.nodeIds())
    {
        auto row = _indexes->at(nodeId);

        QJsonObject index;
        index["nodeId"] = static_cast<int>(nodeId);
        index["row"] = static_cast<int>(row);

        jsonArray.append(index);
    }

    jsonObject["indexes"] = jsonArray;

    return jsonObject;
}

bool UserNodeData::load(const QJsonObject& jsonObject, const ProgressFn& progressFn)
{
    if(!UserData::load(jsonObject, progressFn))
        return false;

    _indexes->resetElements();
    _rowToNodeIdMap.clear();

    if(!jsonObject.contains("indexes") || !jsonObject["indexes"].isArray())
        return false;

    const auto& indexes = jsonObject["indexes"].toArray();

    for(const auto& index : indexes)
    {
        if(!index.isObject())
            return false;

        const auto& indexObject = index.toObject();

        NodeId nodeId = indexObject["nodeId"].toInt();
        size_t row = indexObject["row"].toInt();

        setNodeIdForRowIndex(nodeId, row);
    }

    return true;
}
