#ifndef SELECTNODESCOMMAND_H
#define SELECTNODESCOMMAND_H

#include "shared/commands/icommand.h"

#include "shared/graph/elementid.h"

#include "ui/selectionmanager.h"

#include <memory>
#include <array>

template<typename C>
class SelectNodesCommand : public ICommand
{
private:
    SelectionManager* _selectionManager;
    C _nodeIds;
    bool _clearSelectionFirst;
    QString _pastParticiple;

    NodeId nodeId() const
    {
        Q_ASSERT(_nodeIds.size() == 1);
        return *_nodeIds.begin();
    }

public:
    SelectNodesCommand(SelectionManager* selectionManager, C nodeIds,
                       bool clearSelectionFirst = true) :
        _selectionManager(selectionManager),
        _nodeIds(std::move(nodeIds)),
        _clearSelectionFirst(clearSelectionFirst)
    {}

    QString description() const override
    {
        if(_nodeIds.size() == 1)
        {
            return _selectionManager->nodeIsSelected(nodeId()) ?
                        QObject::tr("Deselect Node") :
                        QObject::tr("Select Node");
        }

        return QObject::tr("Select Nodes");
    }

    QString verb() const override
    {
        if(_nodeIds.size() == 1)
        {
            return _selectionManager->nodeIsSelected(nodeId()) ?
                        QObject::tr("Deselecting Node") :
                        QObject::tr("Selecting Node");
        }

        return QObject::tr("Selecting Nodes");
    }

    QString pastParticiple() const override { return _pastParticiple; }

    bool execute() override
    {
        if(_clearSelectionFirst)
        {
            _selectionManager->suppressSignals();
            _selectionManager->clearNodeSelection();
        }

        if(_nodeIds.size() > 1)
        {
            bool nodesSelected = _selectionManager->selectNodes(_nodeIds);
            _pastParticiple = _selectionManager->numNodesSelectedAsString();
            return nodesSelected;
        }

        bool nodeSelected = _selectionManager->toggleNode(nodeId());
        if(nodeSelected)
            _pastParticiple = _selectionManager->numNodesSelectedAsString();

        return true;
}
};

template<typename C>
auto makeSelectNodesCommand(SelectionManager* selectionManager, C nodeIds, bool clearSelectionFirst = true)
{
    return std::make_unique<SelectNodesCommand<C>>(selectionManager, nodeIds, clearSelectionFirst);
}

inline auto makeSelectNodeCommand(SelectionManager* selectionManager, NodeId nodeId, bool clearSelectionFirst = true)
{
    std::vector<NodeId> nodeIds{{nodeId}};
    return makeSelectNodesCommand(selectionManager, nodeIds, clearSelectionFirst);
}

#endif // SELECTNODESCOMMAND_H
