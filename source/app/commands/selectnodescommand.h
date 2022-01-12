/* Copyright © 2013-2022 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SELECTNODESCOMMAND_H
#define SELECTNODESCOMMAND_H

#include "shared/commands/icommand.h"

#include "shared/graph/elementid.h"
#include "shared/utils/flags.h"

#include "ui/selectionmanager.h"

#include <memory>
#include <array>

enum class SelectNodesClear
{
    None        = 0x0,
    Selection   = 0x1,
    Mask        = 0x2,
    SelectionAndMask = Selection | Mask
};

template<typename C>
class SelectNodesCommand : public ICommand
{
private:
    SelectionManager* _selectionManager;
    C _nodeIds;
    Flags<SelectNodesClear> _clearType;
    QString _pastParticiple;

    NodeId nodeId() const
    {
        Q_ASSERT(_nodeIds.size() == 1);
        return *_nodeIds.begin();
    }

public:
    SelectNodesCommand(SelectionManager* selectionManager, C nodeIds,
                       Flags<SelectNodesClear> clearType) :
        _selectionManager(selectionManager),
        _nodeIds(std::move(nodeIds)),
        _clearType(clearType)
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
        if(_clearType.test(SelectNodesClear::Selection))
        {
            _selectionManager->suppressSignals();
            _selectionManager->clearNodeSelection();
        }

        if(_clearType.test(SelectNodesClear::Mask))
            _selectionManager->clearNodesMask();

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
auto makeSelectNodesCommand(SelectionManager* selectionManager, C nodeIds,
    Flags<SelectNodesClear> clearType = SelectNodesClear::Selection)
{
    return std::make_unique<SelectNodesCommand<C>>(selectionManager, nodeIds, clearType);
}

inline auto makeSelectNodeCommand(SelectionManager* selectionManager, NodeId nodeId,
    Flags<SelectNodesClear> clearType = SelectNodesClear::Selection)
{
    std::vector<NodeId> nodeIds{{nodeId}};
    return makeSelectNodesCommand(selectionManager, nodeIds, clearType);
}

#endif // SELECTNODESCOMMAND_H
