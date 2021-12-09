/* Copyright © 2013-2021 Graphia Technologies Ltd.
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

#include "editattributetablemodel.h"

#include "ui/document.h"
#include "ui/selectionmanager.h"

#include "shared/graph/igraphmodel.h"
#include "shared/attributes/iattribute.h"

#include "shared/utils/container.h"
#include "shared/utils/static_block.h"

#include <QQmlEngine>

int EditAttributeTableModel::rowCount(const QModelIndex&) const
{
    if(_attribute == nullptr)
        return 0;

    switch(_attribute->elementType())
    {
    case ElementType::Node:
        return !_selectedNodes.empty() ? static_cast<int>(_selectedNodes.size()) :
            _document->graphModel()->graph().numNodes();

    case ElementType::Edge: return _document->graphModel()->graph().numEdges();
    default: return 0;
    }
}

int EditAttributeTableModel::columnCount(const QModelIndex&) const
{
    return 2;
}

QVariant EditAttributeTableModel::data(const QModelIndex& index, int role) const
{
    auto column = index.column();
    auto row = index.row();

    NodeId nodeId;
    EdgeId edgeId;

    if(_attribute->elementType() == ElementType::Node)
        nodeId = rowToNodeId(row);
    else if(_attribute->elementType() == ElementType::Edge)
        edgeId = _document->graphModel()->graph().edgeIds().at(static_cast<size_t>(row));

    switch(role)
    {
    case LabelRole:     column = 0; break;
    case AttributeRole: column = 1; break;
    case EditedRole:
        if(_attribute->elementType() == ElementType::Node) return u::contains(_edits._nodeValues, nodeId);
        if(_attribute->elementType() == ElementType::Edge) return u::contains(_edits._edgeValues, edgeId);
        break;
    }

    if(_attribute->elementType() == ElementType::Node)
    {
        const auto& value = u::contains(_edits._nodeValues, nodeId) ? _edits._nodeValues.at(nodeId) : _attribute->valueOf(nodeId);
        return column == 0 ? _document->graphModel()->nodeName(nodeId) : value;
    }
    else if(_attribute->elementType() == ElementType::Edge)
    {
        const auto& value = u::contains(_edits._edgeValues, edgeId) ? _edits._edgeValues.at(edgeId) : _attribute->valueOf(edgeId);
        return column == 0 ? static_cast<int>(edgeId) : value;
    }

    return {};
}

void EditAttributeTableModel::editValue(int row, const QString& value)
{
    if(_attribute->elementType() == ElementType::Node)
    {
        auto nodeId = rowToNodeId(row);

        if(value != _attribute->valueOf(nodeId))
            _edits._nodeValues[nodeId] = value;
    }
    else if(_attribute->elementType() == ElementType::Edge)
    {
        auto edgeId = _document->graphModel()->graph().edgeIds().at(static_cast<size_t>(row));

        if(value != _attribute->valueOf(edgeId))
            _edits._edgeValues[edgeId] = value;
    }

    emit dataChanged(index(row, 1), index(row, 1), {Qt::DisplayRole, AttributeRole, EditedRole});
    emit hasEditsChanged();
    emit editsChanged();
}

void EditAttributeTableModel::resetRowValue(int row)
{
    size_t n = 0;

    if(_attribute->elementType() == ElementType::Node)
    {
        auto nodeId = rowToNodeId(row);

        if(u::contains(_edits._nodeValues, nodeId))
            n = _edits._nodeValues.erase(nodeId);
    }
    else if(_attribute->elementType() == ElementType::Edge)
    {
        auto edgeId = _document->graphModel()->graph().edgeIds().at(static_cast<size_t>(row));

        if(u::contains(_edits._edgeValues, edgeId))
            n = _edits._edgeValues.erase(edgeId);
    }

    if(n > 0)
    {
        emit dataChanged(index(row, 1), index(row, 1), {Qt::DisplayRole, AttributeRole, EditedRole});
        emit hasEditsChanged();
        emit editsChanged();
    }
}

void EditAttributeTableModel::resetAllEdits()
{
    if(!hasEdits())
        return;

    beginResetModel();
    _edits._nodeValues.clear();
    _edits._edgeValues.clear();
    endResetModel();

    emit hasEditsChanged();
    emit editsChanged();
}

bool EditAttributeTableModel::rowIsEdited(int row)
{
    if(_attribute == nullptr)
        return false;

    if(_attribute->elementType() == ElementType::Node)
        return u::contains(_edits._nodeValues, rowToNodeId(row));

    if(_attribute->elementType() == ElementType::Edge)
    {
        auto edgeId = _document->graphModel()->graph().edgeIds().at(static_cast<size_t>(row));
        return u::contains(_edits._edgeValues, edgeId);
    }

    return false;
}

NodeId EditAttributeTableModel::rowToNodeId(int row) const
{
    if(_attribute->elementType() != ElementType::Node)
    {
        qDebug() << "EditAttributeTableModel::rowToNodeId called with non-node attribute";
        return {};
    }

    const auto& v = !_selectedNodes.empty() ? _selectedNodes :
        _document->graphModel()->graph().nodeIds();

    auto index = static_cast<size_t>(row);
    if(index < v.size())
        return v.at(index);

    return {};
}

void EditAttributeTableModel::setDocument(Document* document)
{
    if(document == _document)
        return;

    setAttributeName({});

    if(_document != nullptr)
    {
        disconnect(_document, &Document::selectedNodeIdsChanged,
            this, &EditAttributeTableModel::onSelectionChanged);
    }

    _document = document;

    if(_document != nullptr)
    {
        connect(_document, &Document::selectedNodeIdsChanged,
            this, &EditAttributeTableModel::onSelectionChanged);

        onSelectionChanged();
    }
    else
        _selectedNodes.clear();

    emit documentChanged();
}

void EditAttributeTableModel::setAttributeName(const QString& attributeName)
{
    if(attributeName == _attributeName)
        return;

    beginResetModel();

    _attributeName = attributeName;
    _attribute = !attributeName.isEmpty() ?
        _document->graphModel()->attributeByName(attributeName) : nullptr;

    _edits._nodeValues.clear();
    _edits._edgeValues.clear();

    endResetModel();

    emit attributeNameChanged();
    emit hasEditsChanged();
    emit editsChanged();
}

void EditAttributeTableModel::onSelectionChanged()
{
    beginResetModel();

    _selectedNodes.clear();

    if(_document->selectionManager() != nullptr)
    {
        const auto& selectedNodes = _document->selectionManager()->selectedNodes();

        _selectedNodes.reserve(selectedNodes.size());

        _selectedNodes.insert(_selectedNodes.begin(),
            selectedNodes.begin(), selectedNodes.end());
        std::sort(_selectedNodes.begin(), _selectedNodes.end());
    }

    endResetModel();
}

static_block
{
    qmlRegisterType<EditAttributeTableModel>(APP_URI, APP_MAJOR_VERSION, APP_MINOR_VERSION, "EditAttributeTableModel");
}
