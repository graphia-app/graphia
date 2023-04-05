/* Copyright © 2013-2023 Graphia Technologies Ltd.
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

#ifndef IDOCUMENT_H
#define IDOCUMENT_H

#include "shared/graph/elementid.h"
#include "shared/graph/elementid_containers.h"
#include "shared/utils/flags.h"

#include <vector>

class IGraphModel;
class ISelectionManager;
class ICommandManager;
class QString;

// These values shadow the QMessageBox ones
enum class MessageBoxIcon
{
    NoIcon      = 0,
    Question    = 4,
    Information = 1,
    Warning     = 2,
    Critical    = 3 // clazy:exclude=unexpected-flag-enumerator-value
};

enum class MessageBoxButton
{
    None    = 0,
    Ok      = 0x00000400,
    Cancel  = 0x00400000,
    Close   = 0x00200000,
    Yes     = 0x00004000,
    No      = 0x00010000,
    Abort   = 0x00040000,
    Retry   = 0x00080000,
    Ignore  = 0x00100000
};

class IDocument
{
public:
    virtual ~IDocument() = default;

    virtual const IGraphModel* graphModel() const = 0;
    virtual IGraphModel* graphModel() = 0;

    virtual const ISelectionManager* selectionManager() const = 0;
    virtual ISelectionManager* selectionManager() = 0;

    virtual const ICommandManager* commandManager() const = 0;
    virtual ICommandManager* commandManager() = 0;

    virtual MessageBoxButton messageBox(MessageBoxIcon icon, const QString& title, const QString& text,
        Flags<MessageBoxButton> buttons = MessageBoxButton::Ok) = 0;

    virtual void moveFocusToNode(NodeId nodeId) = 0;
    virtual void moveFocusToNodes(const std::vector<NodeId>& nodeIds) = 0;

    virtual void clearSelectedNodes() = 0;
    virtual void selectNodes(const NodeIdSet& nodeIds) = 0;

    virtual void clearHighlightedNodes() = 0;
    virtual void highlightNodes(const NodeIdSet& nodeIds) = 0;

    virtual void reportProblem(const QString& description) const = 0;

    virtual const QString& log() const = 0;
    virtual void setLog(const QString& log) = 0;
};

#endif // IDOCUMENT_H
