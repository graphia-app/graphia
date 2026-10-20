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

#ifndef TESTDOCUMENT_H
#define TESTDOCUMENT_H

#include "tests/common/testgraphmodel.h"

#include "app/commands/commandmanager.h"
#include "app/ui/selectionmanager.h"

#include "shared/ui/idocument.h"

#include <QString>
#include <QStringList>

#include <vector>

// An IDocument, as plugins see the Document, for a TestGraphModel; what would be
// shown to the user, or would move the camera, is recorded instead
class TestDocument : public IDocument
{
private:
    TestGraphModel* _model;
    SelectionManager _selectionManager;
    CommandManager _commandManager;
    QString _log;

public:
    std::vector<NodeId> _focusedNodeIds;
    NodeIdSet _highlightedNodeIds;
    QStringList _messages;
    mutable QStringList _problems;

    explicit TestDocument(TestGraphModel& model) :
        _model(&model), _selectionManager(model.graphModel())
    {}

    const IGraphModel* graphModel() const override { return &_model->graphModel(); }
    IGraphModel* graphModel() override { return &_model->graphModel(); }

    const SelectionManager* selectionManager() const override { return &_selectionManager; }
    SelectionManager* selectionManager() override { return &_selectionManager; }

    const ICommandManager* commandManager() const override { return &_commandManager; }
    ICommandManager* commandManager() override { return &_commandManager; }

    MessageBoxButton messageBox(MessageBoxIcon, const QString& title, const QString& text,
        Flags<MessageBoxButton> buttons = MessageBoxButton::Ok) override
    {
        _messages.append(title + QStringLiteral(": ") + text);
        return buttons.test(MessageBoxButton::Ok) ? MessageBoxButton::Ok : MessageBoxButton::None;
    }

    void moveFocusToNode(NodeId nodeId) override { _focusedNodeIds = {nodeId}; }
    void moveFocusToNodes(const std::vector<NodeId>& nodeIds) override { _focusedNodeIds = nodeIds; }

    void clearSelectedNodes() override { _selectionManager.clearNodeSelection(); }
    void selectNodes(const NodeIdSet& nodeIds) override
    {
        _selectionManager.clearNodeSelection();
        _selectionManager.selectNodes(nodeIds);
    }

    void clearHighlightedNodes() override { _highlightedNodeIds.clear(); }
    void highlightNodes(const NodeIdSet& nodeIds) override { _highlightedNodeIds = nodeIds; }

    void reportProblem(const QString& description) const override
    {
        _problems.append(description);
    }

    const QString& log() const override { return _log; }
    void setLog(const QString& log) override { _log = log; }
};

#endif // TESTDOCUMENT_H
