#ifndef USERNODEDATA_H
#define USERNODEDATA_H

#include "userdata.h"

#include <map>
#include <memory>
#include "shared/graph/grapharray.h"

class IMutableGraph;
class IGraphModel;

class UserNodeData : public UserData
{
    Q_OBJECT

private:
    std::unique_ptr<NodeArray<size_t>> _indexes;
    std::map<size_t, NodeId> _rowToNodeIdMap;

public:
    void initialise(IMutableGraph& mutableGraph);

    void addNodeId(NodeId nodeId);
    void setNodeIdForRowIndex(NodeId nodeId, size_t row);
    size_t rowIndexForNodeId(NodeId nodeId) const;
    NodeId nodeIdForRowIndex(size_t row) const;

    void setValueByNodeId(NodeId nodeId, const QString& name, const QString& value);
    QVariant valueByNodeId(NodeId nodeId, const QString& name) const;

    void setNodeNamesToFirstUserDataVector(IGraphModel& graphModel);
    void exposeAsAttributes(IGraphModel& graphModel);

    json save(const IMutableGraph& graph, const ProgressFn& progressFn) const;
    bool load(const json& jsonObject, const ProgressFn& progressFn);
};

#endif // USERNODEDATA_H
