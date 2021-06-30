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

#include "nodeattributetablemodel.h"

#include "shared/ui/iselectionmanager.h"
#include "shared/graph/igraphmodel.h"
#include "shared/graph/igraph.h"
#include "shared/attributes/iattribute.h"
#include "shared/attributes/valuetype.h"

#include "shared/utils/container.h"
#include "shared/utils/string.h"

#include <QSet>
#include <QtGlobal>

void NodeAttributeTableModel::initialise(IDocument* document, IUserNodeData* userNodeData)
{
    _roleNames.insert(Roles::NodeIdRole, "nodeId");
    _roleNames.insert(Roles::NodeSelectedRole, "nodeSelected");
    _roleNames.insert(Qt::DisplayRole, "display");

    _document = document;
    _userNodeData = userNodeData;
    _graph = &_document->graphModel()->graph();

    const auto* graphModel = _document->graphModel();

    updateColumnNames();

    const auto* modelQObject = dynamic_cast<const QObject*>(graphModel);
    connect(modelQObject, SIGNAL(attributesChanged(QStringList,QStringList,QStringList)),
            this, SLOT(onAttributesChanged(QStringList,QStringList,QStringList)), Qt::DirectConnection);

    const auto* graphQObject = dynamic_cast<const QObject*>(&graphModel->graph());
    connect(graphQObject, SIGNAL(graphChanged(const Graph*,bool)),
            this, SLOT(onGraphChanged(const Graph*,bool)), Qt::DirectConnection);
}

QStringList NodeAttributeTableModel::columnNames() const
{
    // First make a list from the user data that has been exposed as attributes; the only
    // reason we do this instead of directly using the general list of attributes (below)
    // is because we want to preserve the order in which the attributes were created
    QStringList list = u::toQStringList(_userNodeData->exposedAttributeNames());

    for(auto& attributeName : _document->graphModel()->attributeNames(ElementType::Node))
    {
        const auto* attribute = _document->graphModel()->attributeByName(attributeName);
        Q_ASSERT(attribute != nullptr);

        // Skip the ones that should have already been added or we can't display in a table
        if(attribute->userDefined() || attribute->hasParameter())
            continue;

        Q_ASSERT(!u::contains(list, attributeName));
        list.append(attributeName);
    }

    return list;
}

int NodeAttributeTableModel::indexForColumnName(const QString& columnName)
{
    auto index = _columnNames.indexOf(columnName);
    Q_ASSERT(index >= 0);
    return index;
}

QVariant NodeAttributeTableModel::dataValue(size_t row, const QString& columnName) const
{
    const auto* attribute = _document->graphModel()->attributeByName(columnName);
    if(attribute != nullptr && attribute->isValid())
    {
        Q_ASSERT(attribute->elementType() == ElementType::Node);

        auto nodeId = _userNodeData->elementIdForIndex(row);
        if(!attribute->valueMissingOf(nodeId))
            return attribute->valueOf(nodeId);
    }

    return {};
}

void NodeAttributeTableModel::updateColumnNames()
{
    auto newColumnNames = columnNames();
    if(newColumnNames != _columnNames)
    {
        _columnNames = columnNames();
        Q_ASSERT(u::hasUniqueValues(u::toQStringVector(_columnNames)));
        emit columnNamesChanged();
    }
}

void NodeAttributeTableModel::updateColumn(int role, NodeAttributeTableModel::Column& column,
    const QString& columnName)
{
    column.resize(static_cast<size_t>(rowCount()));

    for(size_t row = 0; row < static_cast<size_t>(rowCount()); row++)
    {
        NodeId nodeId = _userNodeData->elementIdForIndex(row);

        if(nodeId.isNull() || !_document->graphModel()->graph().containsNodeId(nodeId))
        {
            // The graph doesn't necessarily have a node for every row since
            // it may have been transformed, leaving empty rows
            column[row] = {};
        }
        else if(role == Roles::NodeIdRole)
            column[row] = static_cast<int>(nodeId);
        else if(role == Roles::NodeSelectedRole)
            column[row] = _document->selectionManager()->nodeIsSelected(nodeId);
        else
            column[row] = dataValue(row, columnName);
    }
}

void NodeAttributeTableModel::update()
{
    std::unique_lock<std::recursive_mutex> lock(_updateMutex);

    _pendingData.clear();

    updateColumn(Roles::NodeSelectedRole, _nodeSelectedColumn);
    updateColumn(Roles::NodeIdRole, _nodeIdColumn);

    for(const auto& columnName : std::as_const(_columnNames))
    {
        _pendingData.emplace_back(rowCount());
        updateColumn(Qt::DisplayRole, _pendingData.back(), columnName);
    }

    QMetaObject::invokeMethod(this, "onUpdateComplete");
}

void NodeAttributeTableModel::onUpdateComplete()
{
    std::unique_lock<std::recursive_mutex> lock(_updateMutex);

    //FIXME this could probably be made more targetted; in the general case only
    // a subset of _data will actually be updated, in which case we don't need
    // to reset the entire model
    beginResetModel();
    _data = _pendingData;
    endResetModel();

    //FIXME is this actually necessary, in addition to
    // the emit in NodeAttributeTableModel::updateColumnNames()?
    emit columnNamesChanged();
}

void NodeAttributeTableModel::onGraphChanged(const Graph*, bool changeOccurred)
{
    if(changeOccurred)
    {
        _graph->setPhase(tr("Attribute Table"));
        update();
        _graph->clearPhase();
    }
}

bool NodeAttributeTableModel::columnIsCalculated(const QString& columnName) const
{
    const auto* graphModel = _document->graphModel();
    const auto* attribute = graphModel->attributeByName(columnName);

    if(attribute != nullptr && attribute->isValid())
        return !attribute->userDefined();

    return true;
}

bool NodeAttributeTableModel::columnIsHiddenByDefault(const QString&) const
{
    return false;
}

void NodeAttributeTableModel::moveFocusToNodeForRowIndex(size_t row)
{
    auto nodeId = _userNodeData->elementIdForIndex(row);
    _document->moveFocusToNode(nodeId);
}

bool NodeAttributeTableModel::columnIsFloatingPoint(const QString& columnName) const
{
    const auto* graphModel = _document->graphModel();
    const auto* attribute = graphModel->attributeByName(columnName);

    if(attribute != nullptr && attribute->isValid())
        return attribute->valueType() == ValueType::Float;

    return false;
}

bool NodeAttributeTableModel::columnIsNumerical(const QString& columnName) const
{
    const auto* graphModel = _document->graphModel();
    const auto* attribute = graphModel->attributeByName(columnName);

    if(attribute != nullptr && attribute->isValid())
        return attribute->valueType() & ValueType::Numerical;

    return false;
}

bool NodeAttributeTableModel::rowVisible(size_t row) const
{
    Q_ASSERT(row < _nodeSelectedColumn.size());
    return _nodeSelectedColumn[row].toBool();
}

QString NodeAttributeTableModel::columnNameFor(size_t column) const
{
    Q_ASSERT(column < static_cast<size_t>(_columnNames.size()));
    return _columnNames.at(static_cast<int>(column));
}

void NodeAttributeTableModel::onAttributesChanged(QStringList added, QStringList removed, QStringList changed)
{
    std::unique_lock<std::recursive_mutex> lock(_updateMutex);

    auto attributeIneligible = [this](const auto& name)
    {
        const auto* attribute = _document->graphModel()->attributeByName(name);
        return attribute->elementType() != ElementType::Node || attribute->hasParameter();
    };

    // Ignore any non-node attributes, or attributes with parameters
    added.erase(std::remove_if(added.begin(), added.end(), attributeIneligible), added.end());
    changed.erase(std::remove_if(changed.begin(), changed.end(), attributeIneligible), changed.end());
    // (We don't do any filtering on removed as the attributes don't exist to be queried)

    QSet<QString> addedSet(added.begin(), added.end());
    QSet<QString> removedSet(removed.begin(), removed.end());
    QSet<QString> changedSet(changed.begin(), changed.end());

    // Identify any attributes that have been simultaneously added and removed
    auto addedAndRemovedSet = addedSet;
    addedAndRemovedSet.intersect(removedSet);

    // Move them to the set of changed attributes
    addedSet.subtract(addedAndRemovedSet);
    removedSet.subtract(addedAndRemovedSet);
    changedSet.unite(addedAndRemovedSet);

    // Either no attributes are being added, or the ones that are are not in the table already
    Q_ASSERT(addedSet.isEmpty() || !addedSet.intersects({_columnNames.begin(), _columnNames.end()}));

    // Ignore attribute names that aren't in the table
    removedSet.intersect({_columnNames.cbegin(), _columnNames.cend()});

    auto indicesForColumnNames = [this](const auto& names, auto compare)
    {
        std::vector<size_t> indices;
        std::transform(names.begin(), names.end(), std::back_inserter(indices),
            [this](const auto& name) { return static_cast<size_t>(indexForColumnName(name)); });
        std::sort(indices.begin(), indices.end(), compare);
        return indices;
    };

    std::vector<size_t> changedIndices = indicesForColumnNames(changedSet, std::less<>());
    for(size_t index : changedIndices)
    {
        const auto& columnName = _columnNames.at(static_cast<int>(index));
        auto& column = _pendingData.at(index);
        updateColumn(Qt::DisplayRole, column, columnName);
    }

    std::vector<size_t> removedIndices = indicesForColumnNames(removedSet, std::greater<>());
    for(size_t index : removedIndices)
    {
        Q_ASSERT(index < _pendingData.size());
        _pendingData.erase(_pendingData.begin() + static_cast<int>(index));
    }

    updateColumnNames();

    std::vector<size_t> addedIndices = indicesForColumnNames(addedSet, std::less<>());
    for(size_t index : addedIndices)
    {
        const auto& columnName = _columnNames.at(static_cast<int>(index));

        if(index < _pendingData.size())
            _pendingData.insert(_pendingData.begin() + static_cast<int>(index), {{}});
        else
            _pendingData.resize(index + 1);

        auto& column = _pendingData.at(index);
        updateColumn(Qt::DisplayRole, column, columnName);
    }

    if(!addedIndices.empty() || !removedIndices.empty() || !changedIndices.empty())
        QMetaObject::invokeMethod(this, "onUpdateComplete");
}

int NodeAttributeTableModel::rowCount(const QModelIndex&) const
{
    return static_cast<int>(_userNodeData->numValues());
}

int NodeAttributeTableModel::columnCount(const QModelIndex&) const
{
    return _columnNames.size();
}

QVariant NodeAttributeTableModel::data(const QModelIndex& index, int role) const
{
    auto column = static_cast<size_t>(index.column());
    if(role == Qt::DisplayRole)
    {
        if(column >= _data.size())
            return {};

        const auto& dataColumn = _data.at(column);

        auto row = static_cast<size_t>(index.row());
        if(row >= dataColumn.size())
            return {};

        auto cachedValue = dataColumn.at(row);
        return cachedValue;
    }

    if(role == Roles::NodeSelectedRole && !_nodeSelectedColumn.empty())
    {
        auto row = static_cast<size_t>(index.row());
        return _nodeSelectedColumn.at(row);
    }
    return {};
}

void NodeAttributeTableModel::onSelectionChanged()
{
    updateColumn(Roles::NodeSelectedRole, _nodeSelectedColumn);
    emit selectionChanged();
}
