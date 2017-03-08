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
    std::unique_ptr<NodeArray<int>> _indexes;
    std::map<int, NodeId> _rowToNodeIdMap;

public:
    void initialise(IMutableGraph& mutableGraph);

    void addNodeId(NodeId nodeId);
    void setNodeIdForRowIndex(NodeId nodeId, int row);
    int rowIndexForNodeId(NodeId nodeId) const;
    NodeId nodeIdForRowIndex(int row) const;

    void setValueByNodeId(NodeId nodeId, const QString& name, const QString& value);
    QString valueByNodeId(NodeId nodeId, const QString& name) const;

    void setNodeNamesToFirstUserDataVector(IGraphModel& graphModel);
    void exposeToGraphModel(IGraphModel& graphModel);
};

#endif // USERNODEDATA_H
