#ifndef SELECTNODESCOMMAND_H
#define SELECTNODESCOMMAND_H

#include "command.h"

#include "shared/graph/elementid.h"

#include "../ui/selectionmanager.h"

#include <memory>
#include <vector>

template<typename C>
class SelectNodesCommand : public Command
{
private:
    SelectionManager* _selectionManager;
    C _nodeIds;
    bool _toggle;
    bool _nodeWasSelected;

    NodeId nodeId() const
    {
        Q_ASSERT(_nodeIds.size() == 1);
        return *_nodeIds.begin();
    }

public:
    SelectNodesCommand(SelectionManager* selectionManager, C nodeIds, bool toggle = false) :
        Command(), _selectionManager(selectionManager), _nodeIds(nodeIds), _toggle(toggle)
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
        if(_nodeIds.size() > 1)
        {
            bool nodesSelected = _selectionManager->selectNodes(_nodeIds);
            setPastParticiple(_selectionManager->numNodesSelectedAsString());
            return nodesSelected;
        }
        else
        {
            if(!_toggle)
                _selectionManager->clearNodeSelection();

            _selectionManager->toggleNode(nodeId());

            if(!_nodeWasSelected)
                setPastParticiple(_selectionManager->numNodesSelectedAsString());

            return true;
        }
    }
};

template<typename C>
auto makeSelectNodesCommand(SelectionManager* selectionManager, C nodeIds, bool toggle = false)
{
    return std::make_shared<SelectNodesCommand<C>>(selectionManager, nodeIds, toggle);
}

// This doesn't really need to be a template, but the alternative is defining it in its own
// compilation unit, which gets a bit awkward
template<typename T>
auto makeSelectNodeCommand(SelectionManager* selectionManager, T nodeId, bool toggle = false)
{
    std::vector<T> nodeIds{nodeId};
    return makeSelectNodesCommand(selectionManager, nodeIds, toggle);
}

#endif // SELECTNODESCOMMAND_H
