#ifndef SELECTNODESCOMMAND_H
#define SELECTNODESCOMMAND_H

#include "command.h"

#include "shared/graph/elementid.h"

#include "ui/selectionmanager.h"

#include <memory>
#include <array>

template<typename C>
class SelectNodesCommand : public Command
{
private:
    SelectionManager* _selectionManager;
    C _nodeIds;
    bool _clearSelectionFirst;
    bool _nodeWasSelected;

    NodeId nodeId() const
    {
        Q_ASSERT(_nodeIds.size() == 1);
        return *_nodeIds.begin();
    }

public:
    SelectNodesCommand(SelectionManager* selectionManager, C nodeIds,
                       bool clearSelectionFirst = true) :
        Command(), _selectionManager(selectionManager), _nodeIds(nodeIds),
        _clearSelectionFirst(clearSelectionFirst)
    {
        if(_nodeIds.size() > 1)
        {
            setDescription(QObject::tr("Select Nodes"));
            setVerb(QObject::tr("Selecting Nodes"));
        }
        else
        {
            _nodeWasSelected = _selectionManager->nodeIsSelected(nodeId());

            setDescription(_nodeWasSelected ? QObject::tr("Deselect Node") : QObject::tr("Select Node"));
            setVerb(_nodeWasSelected ? QObject::tr("Deselecting Node") : QObject::tr("Selecting Node"));
        }
    }

    bool execute()
    {
        if(_clearSelectionFirst)
        {
            _selectionManager->suppressSignals();
            _selectionManager->clearNodeSelection();
        }

        if(_nodeIds.size() > 1)
        {
            bool nodesSelected = _selectionManager->selectNodes(_nodeIds);
            setPastParticiple(_selectionManager->numNodesSelectedAsString());
            return nodesSelected;
        }
        else
        {
            _selectionManager->toggleNode(nodeId());

            if(!_nodeWasSelected)
                setPastParticiple(_selectionManager->numNodesSelectedAsString());

            return true;
        }
    }
};

template<typename C>
auto makeSelectNodesCommand(SelectionManager* selectionManager, C nodeIds, bool clearSelectionFirst = true)
{
    return std::make_shared<SelectNodesCommand<C>>(selectionManager, nodeIds, clearSelectionFirst);
}

// This doesn't really need to be a template, but the alternative is defining it in its own
// compilation unit, which gets a bit awkward
template<typename T>
auto makeSelectNodeCommand(SelectionManager* selectionManager, T nodeId, bool clearSelectionFirst = true)
{
    std::array<T, 1> nodeIds{{nodeId}};
    return makeSelectNodesCommand(selectionManager, nodeIds, clearSelectionFirst);
}

#endif // SELECTNODESCOMMAND_H
