/* Copyright © 2013-2024 Graphia Technologies Ltd.
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

#ifndef DELETENODESCOMMAND_H
#define DELETENODESCOMMAND_H

#include "shared/commands/icommand.h"

#include "graph/graph.h"

#include <vector>

class GraphModel;
class SelectionManager;

class DeleteNodesCommand : public ICommand
{
private:
    GraphModel* _graphModel = nullptr;
    SelectionManager* _selectionManager = nullptr;

    bool _multipleNodes = false;
    NodeIdSet _selectedNodeIds;
    NodeIdSet _nodeIds;
    std::vector<Edge> _edges;

public:
    DeleteNodesCommand(GraphModel* graphModel,
                       SelectionManager* selectionManager,
                       NodeIdSet nodeIds);

    QString description() const override;
    QString verb() const override;
    QString pastParticiple() const override;

    bool execute() override;
    void undo() override;
};

#endif // DELETENODESCOMMAND_H
