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

template<typename ElementIds>
static auto rowToElementIds(int row, const IAttribute* attribute, const ElementIds& elementIds)
{
    const auto& sharedValues = attribute->sharedValues();
    const auto& sharedValue = sharedValues.at(static_cast<size_t>(row))._value;

    using E = typename std::remove_reference_t<decltype(elementIds)>::value_type;
    std::vector<E> matches;

    for(auto elementId : elementIds)
    {
        if(attribute->stringValueOf(elementId) == sharedValue)
            matches.emplace_back(elementId);
    }

    return matches;
}

EditAttributeTableModel::EditAttributeTableModel()
{
    connect(this, &EditAttributeTableModel::combineSharedValuesChanged, [this]
    {
        auto hadEdits = hasEdits() && !_combineSharedValues;

        beginResetModel();

        // Reset the edits when going from non-combined to combined (as in that case
        // rows that are combined many have multiple different values)
        if(_combineSharedValues)
        {
            _edits._nodeValues.clear();
            _edits._edgeValues.clear();
        }

        endResetModel();

        if(hadEdits)
        {
            emit hasEditsChanged();
            emit editsChanged();
        }
    });
}

int EditAttributeTableModel::rowCount(const QModelIndex&) const
{
    if(_attribute == nullptr)
        return 0;

    if(_combineSharedValues)
        return static_cast<int>(_attribute->sharedValues().size());

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

    if(role == EditedRole)
        return rowIsEdited(row) && column == 1;

    switch(role)
    {
    case LabelRole:     column = 0; break;
    case AttributeRole: column = 1; break;
    }

    if(_attribute == nullptr)
        return {};

    if(_combineSharedValues)
    {
        const auto& sharedValues = _attribute->sharedValues();
        const auto& sharedValue = sharedValues.at(static_cast<size_t>(row));

        if(column == 0)
            return sharedValue._count;

        if(column == 1)
        {
            if(_attribute->elementType() == ElementType::Node)
            {
                auto nodeIds = rowToElementIds(row, _attribute, _document->graphModel()->graph().nodeIds());

                if(!nodeIds.empty() && u::contains(_edits._nodeValues, nodeIds.front()))
                    return _edits._nodeValues.at(nodeIds.front());
            }
            else if(_attribute->elementType() == ElementType::Edge)
            {
                auto edgeIds = rowToElementIds(row, _attribute, _document->graphModel()->graph().edgeIds());

                if(!edgeIds.empty() && u::contains(_edits._edgeValues, edgeIds.front()))
                    return _edits._edgeValues.at(edgeIds.front());
            }

            return sharedValue._value;
        }
    }
    else
    {
        if(_attribute->elementType() == ElementType::Node)
        {
            auto nodeId = rowToNodeId(row);
            const auto& value = u::contains(_edits._nodeValues, nodeId) ? _edits._nodeValues.at(nodeId) : _attribute->valueOf(nodeId);
            return column == 0 ? _document->graphModel()->nodeName(nodeId) : value;
        }

        if(_attribute->elementType() == ElementType::Edge)
        {
            auto edgeId = _document->graphModel()->graph().edgeIds().at(static_cast<size_t>(row));
            const auto& value = u::contains(_edits._edgeValues, edgeId) ? _edits._edgeValues.at(edgeId) : _attribute->valueOf(edgeId);
            return column == 0 ? static_cast<int>(edgeId) : value;
        }
    }

    return {};
}

void EditAttributeTableModel::editValue(int row, const QString& value)
{
    if(_combineSharedValues)
    {
        if(_attribute->elementType() == ElementType::Node)
        {
            for(auto nodeId : rowToElementIds(row, _attribute, _document->graphModel()->graph().nodeIds()))
                _edits._nodeValues[nodeId] = value;
        }
        else if(_attribute->elementType() == ElementType::Edge)
        {
            for(auto edgeId : rowToElementIds(row, _attribute, _document->graphModel()->graph().edgeIds()))
                _edits._edgeValues[edgeId] = value;
        }
    }
    else
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
    }

    emit dataChanged(index(row, 1), index(row, 1), {Qt::DisplayRole, AttributeRole, EditedRole});
    emit hasEditsChanged();
    emit editsChanged();
}

void EditAttributeTableModel::resetRowValue(int row)
{
    size_t n = 0;

    if(_combineSharedValues)
    {
        if(_attribute->elementType() == ElementType::Node)
        {
            for(auto nodeId : rowToElementIds(row, _attribute, _document->graphModel()->graph().nodeIds()))
                n += _edits._nodeValues.erase(nodeId);
        }
        else if(_attribute->elementType() == ElementType::Edge)
        {
            for(auto edgeId : rowToElementIds(row, _attribute, _document->graphModel()->graph().edgeIds()))
                n += _edits._edgeValues.erase(edgeId);
        }
    }
    else
    {
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

bool EditAttributeTableModel::rowIsEdited(int row) const
{
    if(_attribute == nullptr)
        return false;

    if(_combineSharedValues)
    {
        if(_attribute->elementType() == ElementType::Node)
        {
            auto nodeIds = rowToElementIds(row, _attribute, _document->graphModel()->graph().nodeIds());

            if(!nodeIds.empty())
                return u::contains(_edits._nodeValues, nodeIds.front());
        }
        else if(_attribute->elementType() == ElementType::Edge)
        {
            auto edgeIds = rowToElementIds(row, _attribute, _document->graphModel()->graph().edgeIds());

            if(!edgeIds.empty())
                return u::contains(_edits._edgeValues, edgeIds.front());
        }
    }
    else
    {
        if(_attribute->elementType() == ElementType::Node)
            return u::contains(_edits._nodeValues, rowToNodeId(row));

        if(_attribute->elementType() == ElementType::Edge)
        {
            auto edgeId = _document->graphModel()->graph().edgeIds().at(static_cast<size_t>(row));
            return u::contains(_edits._edgeValues, edgeId);
        }
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
