/* Copyright © 2013-2020 Graphia Technologies Ltd.
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
    connect(modelQObject, SIGNAL(attributesChanged(const QStringList&, const QStringList&, const QStringList&)),
            this, SLOT(onAttributesChanged(const QStringList&, const QStringList&, const QStringList&)), Qt::DirectConnection);

    const auto* graphQObject = dynamic_cast<const QObject*>(&graphModel->graph());
    connect(graphQObject, SIGNAL(graphChanged(const Graph*, bool)),
            this, SLOT(onGraphChanged(const Graph*, bool)), Qt::DirectConnection);
}

QStringList NodeAttributeTableModel::columnNames() const
{
    QStringList list = u::toQStringList(_userNodeData->vectorNames());

    for(auto& attributeName : _document->graphModel()->attributeNames(ElementType::Node))
    {
        const auto* attribute = _document->graphModel()->attributeByName(attributeName);
        Q_ASSERT(attribute != nullptr);

        // We can't show parameterised attributes in the table
        if(attribute->hasParameter())
            continue;

        if(!u::contains(list, attributeName))
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
    _columnNames = columnNames();
    _columnCount = _columnNames.size();
    Q_ASSERT(u::hasUniqueValues(u::toQStringVector(_columnNames)));
    emit columnNamesChanged();
}

void NodeAttributeTableModel::updateAttribute(const QString& attributeName)
{
    std::unique_lock<std::recursive_mutex> lock(_updateMutex);

    auto index = static_cast<size_t>(indexForColumnName(attributeName));

    Q_ASSERT(index < _pendingData.size());
    auto& column = _pendingData.at(index);

    updateColumn(Qt::DisplayRole, column, attributeName);

    QMetaObject::invokeMethod(this, "onUpdateColumnComplete", Q_ARG(QString, attributeName));
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

    for(const auto& columnName : _columnNames)
    {
        _pendingData.emplace_back(rowCount());
        updateColumn(Qt::DisplayRole, _pendingData.back(), columnName);
    }

    QMetaObject::invokeMethod(this, "onUpdateComplete");
}

void NodeAttributeTableModel::onUpdateColumnComplete(const QString& columnName)
{
    std::unique_lock<std::recursive_mutex> lock(_updateMutex);

    emit layoutAboutToBeChanged();
    auto column = static_cast<size_t>(indexForColumnName(columnName));
    _data.at(column) = _pendingData.at(column);

    //FIXME: FIXME: This comment is out of date and refers to TableView 1
    //FIXME: Emitting dataChanged /should/ be faster than doing a layoutChanged, but
    // for some reason it's not, even with https://codereview.qt-project.org/#/c/219278/
    // applied. Most of the performance seems to be lost deep in TableView's JS so perhaps
    // we should just ditch TableView and implement our own custom table, that we can
    // control better. Certainly the internet suggests using ListView:
    //      https://stackoverflow.com/a/43856015
    //      https://stackoverflow.com/a/45188582
    // Also, note that NodeAttributeTableView currently relies on layoutChanged, so if
    // what we emit changes, we need to account for it there too.
    //emit dataChanged(index(0, column), index(rowCount() - 1, column), {role});

    emit layoutChanged();
}

void NodeAttributeTableModel::onUpdateComplete()
{
    std::unique_lock<std::recursive_mutex> lock(_updateMutex);

    beginResetModel();
    _data = _pendingData;
    endResetModel();
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

    if(!added.isEmpty())
    {
        added.erase(std::remove_if(added.begin(), added.end(),
        [this](const auto& name)
        {
            const auto* attribute = _document->graphModel()->attributeByName(name);
            return attribute->elementType() != ElementType::Node || attribute->hasParameter();
        }), added.end());
    }

    QSet<QString> addedSet(added.begin(), added.end());
    QSet<QString> removedSet(removed.begin(), removed.end());

    // Either no attributes are being added, or the ones that are are not in the table already
    Q_ASSERT(addedSet.isEmpty() || !addedSet.intersects({_columnNames.begin(), _columnNames.end()}));

    // Ignore attribute names that aren't in the table
    removedSet.intersect({_columnNames.begin(), _columnNames.end()});

    if(addedSet.isEmpty() && removedSet.isEmpty())
    {
        // There is no structural change to the table, but some roles' values
        // may have changed, so we need to update these individually
        for(const auto& attribute : changed)
        {
            if(u::contains(_columnNames, attribute))
                updateAttribute(attribute);
        }

        return;
    }

    // We can ignore attributes with changed values from here on out, as any roles requiring
    // an update will be taken care of en-masse, in onUpdateComplete

    std::vector<size_t> removedIndices;
    std::transform(removedSet.begin(), removedSet.end(), std::back_inserter(removedIndices),
        [this](const auto& name) { return static_cast<size_t>(_columnNames.indexOf(name)); });
    std::sort(removedIndices.begin(), removedIndices.end(), std::greater<>());

    for(size_t index : removedIndices)
    {
        Q_ASSERT(index < _pendingData.size());
        _pendingData.erase(_pendingData.begin() + static_cast<int>(index));
    }

    updateColumnNames();

    std::vector<size_t> addedIndices;
    std::transform(addedSet.begin(), addedSet.end(), std::back_inserter(addedIndices),
        [this](const auto& name) { return static_cast<size_t>(_columnNames.indexOf(name)); });
    std::sort(addedIndices.begin(), addedIndices.end(), std::less<>());

    for(size_t index : addedIndices)
    {
        auto columnName = _columnNames.at(static_cast<int>(index));

        if(index < _pendingData.size())
            _pendingData.insert(_pendingData.begin() + static_cast<int>(index), {{}});
        else
            _pendingData.resize(index + 1);

        auto& column = _pendingData.at(index);
        updateColumn(Qt::DisplayRole, column, columnName);
    }

    QMetaObject::invokeMethod(this, "onUpdateComplete");
}

int NodeAttributeTableModel::rowCount(const QModelIndex&) const
{
    return static_cast<int>(_userNodeData->numValues());
}

int NodeAttributeTableModel::columnCount(const QModelIndex&) const
{
    return _columnCount;
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
