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

#ifndef SELECTIONMANAGER_H
#define SELECTIONMANAGER_H

#include "shared/ui/iselectionmanager.h"
#include "shared/utils/container.h"

#include <QObject>

#include <memory>
#include <mutex>

class GraphModel;

class SelectionManager : public QObject, public ISelectionManager
{
    Q_OBJECT
public:
    explicit SelectionManager(const GraphModel& graphModel);

    NodeIdSet selectedNodes() const override;
    NodeIdSet unselectedNodes() const override;

    bool selectNode(NodeId nodeId) override;
    bool selectNodes(const NodeIdSet& nodeIds) override;
    bool selectNodes(const std::vector<NodeId>& nodeIds);

    bool deselectNode(NodeId nodeId) override;
    bool deselectNodes(const NodeIdSet& nodeIds) override;
    bool deselectNodes(const std::vector<NodeId>& nodeIds);

    bool toggleNode(NodeId nodeId);

    bool nodeIsSelected(NodeId nodeId) const override;

    bool selectAllNodes() override;
    bool clearNodeSelection() override;
    void invertNodeSelection() override;

    void setNodesMask(const NodeIdSet& nodeIds, bool applyMask = true);
    void setNodesMask(const std::vector<NodeId>& nodeIds, bool applyMask = true);
    void clearNodesMask() { _nodeIdsMask.clear(); emit nodesMaskChanged(); }
    bool nodesMaskActive() const { return !_nodeIdsMask.empty(); }

    size_t numNodesSelected() const { return _selectedNodeIds.size(); }
    QString numNodesSelectedAsString() const;

    void suppressSignals();

private:
    const GraphModel* _graphModel = nullptr;

    mutable std::recursive_mutex _mutex;
    NodeIdSet _selectedNodeIds;

    // Temporary storage for NodeIds that have been deleted
    std::vector<NodeId> _deletedNodes;

    NodeIdSet _nodeIdsMask;

    bool _suppressSignals = false;

    bool signalsSuppressed();

    template<typename Fn>
    bool callFnAndMaybeEmit(const Fn& fn)
    {
        const std::unique_lock<std::recursive_mutex> lock(_mutex);

        const bool selectionWillChange = fn();

        if(!signalsSuppressed() && selectionWillChange)
            emit selectionChanged(this);

        return selectionWillChange;
    }

signals:
    void selectionChanged(const SelectionManager* selectionManager);
    void nodesMaskChanged();
};

#endif // SELECTIONMANAGER_H
