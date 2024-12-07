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

#ifndef ISELECTIONMANAGER_H
#define ISELECTIONMANAGER_H

#include "shared/graph/elementid.h"
#include "shared/graph/elementid_containers.h"

class ISelectionManager
{
public:
    virtual ~ISelectionManager() = default;

    virtual NodeIdSet selectedNodes() const = 0;
    virtual NodeIdSet unselectedNodes() const = 0;

    virtual bool selectNode(NodeId nodeId) = 0;
    virtual bool selectNodes(const NodeIdSet& nodeIds) = 0;

    virtual bool deselectNode(NodeId nodeId) = 0;
    virtual bool deselectNodes(const NodeIdSet& nodeIds) = 0;

    virtual bool nodeIsSelected(NodeId nodeId) const = 0;

    virtual bool selectAllNodes() = 0;
    virtual bool clearNodeSelection() = 0;
    virtual void invertNodeSelection() = 0;
};

#endif // ISELECTIONMANAGER_H
