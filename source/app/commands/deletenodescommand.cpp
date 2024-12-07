/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

#include "deletenodescommand.h"

#include <QObject>

#include "app/graph/mutablegraph.h"
#include "app/graph/graphmodel.h"
#include "app/ui/selectionmanager.h"

DeleteNodesCommand::DeleteNodesCommand(GraphModel* graphModel,
                                       SelectionManager* selectionManager,
                                       NodeIdSet nodeIds) :
    _graphModel(graphModel),
    _selectionManager(selectionManager),
    _selectedNodeIds(_selectionManager->selectedNodes()),
    _nodeIds(std::move(nodeIds))
{
    _multipleNodes = (_nodeIds.size() > 1);
}

QString DeleteNodesCommand::description() const
{
    return _multipleNodes ? QObject::tr("Delete Nodes") : QObject::tr("Delete Node");
}

QString DeleteNodesCommand::verb() const
{
    return _multipleNodes ? QObject::tr("Deleting Nodes") : QObject::tr("Deleting Node");
}

QString DeleteNodesCommand::pastParticiple() const
{
    return _multipleNodes ? QString(QObject::tr("%1 Nodes Deleted")).arg(_nodeIds.size()) :
                                    QObject::tr("Node Deleted");
}

bool DeleteNodesCommand::execute()
{
    _edges = _graphModel->mutableGraph().edgesForNodeIds(_nodeIds);
    _selectionManager->suppressSignals();
    _selectionManager->deselectNodes(_nodeIds);
    _graphModel->mutableGraph().removeNodes(_nodeIds);
    return true;
}

void DeleteNodesCommand::undo()
{
    _graphModel->mutableGraph().performTransaction(
        [this](IMutableGraph& graph)
        {
            graph.addNodes(_nodeIds);
            graph.addEdges(_edges);
        });

    _selectionManager->selectNodes(_selectedNodeIds);
}
