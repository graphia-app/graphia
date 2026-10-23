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

#include "nodeattributetabledisplaymodel.h"

#include "shared/plugins/nodeattributetablemodel.h"

#include "shared/utils/container.h"
#include "shared/utils/string.h"

#include <QList>
#include <QMetaType>
#include <QtGlobal>

#include <algorithm>
#include <limits>

NodeAttributeTableDisplayModel::NodeAttributeTableDisplayModel(QObject* parent) :
    QAbstractTableModel(parent)
{}

QAbstractItemModel* NodeAttributeTableDisplayModel::sourceModel() const
{
    return _sourceModel;
}

void NodeAttributeTableDisplayModel::setSourceModel(QAbstractItemModel* sourceModel)
{
    auto* nodeAttributeTableModel = qobject_cast<NodeAttributeTableModel*>(sourceModel);
    Q_ASSERT(sourceModel == nullptr || nodeAttributeTableModel != nullptr);

    if(nodeAttributeTableModel == _sourceModel)
        return;

    if(_sourceModel != nullptr)
        disconnect(_sourceModel, nullptr, this, nullptr);

    _sourceModel = nodeAttributeTableModel;

    if(_sourceModel != nullptr)
    {
        connect(_sourceModel, &QAbstractItemModel::modelReset, this, &NodeAttributeTableDisplayModel::onSourceModelReset);
        connect(_sourceModel, &QAbstractItemModel::layoutChanged, this, &NodeAttributeTableDisplayModel::reset);
        connect(_sourceModel, &QAbstractItemModel::dataChanged, this, &NodeAttributeTableDisplayModel::onSourceDataChanged);
        connect(_sourceModel, &NodeAttributeTableModel::selectionChanged, this, &NodeAttributeTableDisplayModel::onSourceSelectionChanged);
    }

    onSourceModelReset();
    emit sourceModelChanged();
}

int NodeAttributeTableDisplayModel::rowCount(const QModelIndex& parent) const
{
    if(parent.isValid())
        return 0;

    return static_cast<int>(_sourceRows.size());
}

int NodeAttributeTableDisplayModel::columnCount(const QModelIndex& parent) const
{
    if(parent.isValid())
        return 0;

    return static_cast<int>(_sourceColumns.size());
}

QVariant NodeAttributeTableDisplayModel::data(const QModelIndex& index, int role) const
{
    if(!index.isValid() || _sourceModel == nullptr)
        return {};

    if(role == SubSelectedRole)
        return isRowSelected(index.row());

    auto row = static_cast<size_t>(index.row());
    auto column = static_cast<size_t>(index.column());

    if(row >= _sourceRows.size() || column >= _sourceColumns.size())
        return {};

    return _sourceModel->data(_sourceModel->index(_sourceRows.at(row), _sourceColumns.at(column)), role);
}

QHash<int, QByteArray> NodeAttributeTableDisplayModel::roleNames() const
{
    QHash<int, QByteArray> roleNames;

    if(_sourceModel != nullptr)
        roleNames = _sourceModel->roleNames();

    roleNames.insert(Roles::SubSelectedRole, "subSelected");
    return roleNames;
}

int NodeAttributeTableDisplayModel::sourceRowFor(int row) const
{
    if(row < 0 || static_cast<size_t>(row) >= _sourceRows.size())
        return -1;

    return _sourceRows.at(static_cast<size_t>(row));
}

int NodeAttributeTableDisplayModel::sourceColumnFor(int column) const
{
    if(column < 0 || static_cast<size_t>(column) >= _sourceColumns.size())
        return -1;

    return _sourceColumns.at(static_cast<size_t>(column));
}

QVariantList NodeAttributeTableDisplayModel::sourceRowsAsVariantList() const
{
    QVariantList sourceRows;
    sourceRows.reserve(static_cast<qsizetype>(_sourceRows.size()));

    for(auto sourceRow : _sourceRows)
        sourceRows.append(sourceRow);

    return sourceRows;
}

bool NodeAttributeTableDisplayModel::isRowSelected(int row) const
{
    if(row < 0 || static_cast<size_t>(row) >= _rowSelected.size())
        return false;

    return _rowSelected.at(static_cast<size_t>(row));
}

template<typename Fn>
void NodeAttributeTableDisplayModel::changeSelection(const Fn& fn)
{
    int firstChangedRow = std::numeric_limits<int>::max();
    int lastChangedRow = -1;

    fn([&](int row, bool selected)
    {
        if(_rowSelected.at(static_cast<size_t>(row)) == selected)
            return;

        _rowSelected.at(static_cast<size_t>(row)) = selected;
        firstChangedRow = std::min(firstChangedRow, row);
        lastChangedRow = std::max(lastChangedRow, row);
    });

    if(lastChangedRow >= 0 && columnCount() > 0)
    {
        emit dataChanged(index(firstChangedRow, 0),
            index(lastChangedRow, columnCount() - 1), { Roles::SubSelectedRole });
    }

    publishSelection();
}

void NodeAttributeTableDisplayModel::publishSelection()
{
    QList<int> selectedRows;

    for(size_t row = 0; row < _rowSelected.size(); row++)
    {
        if(_rowSelected.at(row))
            selectedRows.append(_sourceRows.at(row));
    }

    if(selectedRows != _selectedRows)
    {
        _selectedRows = std::move(selectedRows);
        emit selectedRowsChanged();
    }
}

void NodeAttributeTableDisplayModel::selectRows(int firstRow, int lastRow, bool clearFirst)
{
    changeSelection([&](const auto& setRowSelected)
    {
        if(clearFirst)
        {
            for(int row = 0; row < rowCount(); row++)
                setRowSelected(row, false);
        }

        auto [first, last] = std::minmax(firstRow, lastRow);

        for(int row = std::max(first, 0); row <= std::min(last, rowCount() - 1); row++)
            setRowSelected(row, true);
    });
}

void NodeAttributeTableDisplayModel::deselectRows(int firstRow, int lastRow)
{
    changeSelection([&](const auto& setRowSelected)
    {
        auto [first, last] = std::minmax(firstRow, lastRow);

        for(int row = std::max(first, 0); row <= std::min(last, rowCount() - 1); row++)
            setRowSelected(row, false);
    });
}

void NodeAttributeTableDisplayModel::selectSourceRows(const std::vector<int>& sourceRows, bool clearFirst)
{
    changeSelection([&](const auto& setRowSelected)
    {
        if(clearFirst)
        {
            for(int row = 0; row < rowCount(); row++)
                setRowSelected(row, false);
        }

        for(auto sourceRow : sourceRows)
        {
            if(sourceRow < 0 || static_cast<size_t>(sourceRow) >= _rowForSourceRow.size())
                continue;

            auto row = _rowForSourceRow.at(static_cast<size_t>(sourceRow));
            if(row >= 0)
                setRowSelected(row, true);
        }
    });
}

void NodeAttributeTableDisplayModel::setRowOrder(const std::vector<int>& sourceRows)
{
    _rowOrderMap.clear();

    for(size_t i = 0; i < sourceRows.size(); i++)
        _rowOrderMap[sourceRows[i]] = i;

    resort();
    emit isRowOrderSetChanged();
}

void NodeAttributeTableDisplayModel::setHiddenColumns(const QStringList& hiddenColumns)
{
    _hiddenColumns = hiddenColumns;
    reset();
}

void NodeAttributeTableDisplayModel::setColumnOrder(const QStringList& columnOrder)
{
    // Remove everything from columnOrder that's not in _columnNames
    auto newColumnOrder = u::toQStringList(u::setIntersection(
        static_cast<const QList<QString>&>(columnOrder),
        static_cast<const QList<QString>&>(_columnNames)));

    std::vector<std::pair<int, QString>> newColumns;
    for(int i = 0; i < _columnNames.size(); i++)
    {
        const auto& value = _columnNames.at(i);
        if(!u::contains(newColumnOrder, value))
            newColumns.emplace_back(i, value);
    }

    // Add everything from _columnNames that's not in columnOrder,
    // in the same index as in _columnNames
    for(const auto& newColumn : newColumns)
        newColumnOrder.insert(newColumn.first, newColumn.second);

    if(newColumnOrder != _columnOrder)
    {
        _columnOrder = newColumnOrder;
        reset();
    }
}

QString NodeAttributeTableDisplayModel::sortColumn() const
{
    if(_sortColumnAndOrders.empty())
        return {};

    return _sortColumnAndOrders.front().first;
}

void NodeAttributeTableDisplayModel::setSortColumn(const QString& newSortColumn)
{
    if((newSortColumn.isEmpty() || newSortColumn == sortColumn()) && _rowOrderMap.empty())
        return;

    _rowOrderMap.clear();

    auto currentSortOrder = Qt::AscendingOrder;

    auto existing = std::ranges::find_if(_sortColumnAndOrders,
    [newSortColumn](const auto& value)
    {
        return value.first == newSortColumn;
    });

    // If the column has been sorted on before, remove it so
    // that adding it brings it to the front
    if(existing != _sortColumnAndOrders.end())
    {
        currentSortOrder = existing->second;
        _sortColumnAndOrders.erase(existing);
    }

    _sortColumnAndOrders.emplace_front(newSortColumn, currentSortOrder);

    resort();
    emit sortColumnChanged(newSortColumn);
    emit sortOrderChanged(currentSortOrder);
    emit isRowOrderSetChanged();
}

Qt::SortOrder NodeAttributeTableDisplayModel::sortOrder() const
{
    if(_sortColumnAndOrders.empty())
        return Qt::DescendingOrder;

    return _sortColumnAndOrders.front().second;
}

void NodeAttributeTableDisplayModel::setSortOrder(Qt::SortOrder newSortOrder)
{
    if((_sortColumnAndOrders.empty() || newSortOrder == sortOrder()) && _rowOrderMap.empty())
        return;

    _rowOrderMap.clear();

    if(!_sortColumnAndOrders.empty())
        _sortColumnAndOrders.front().second = newSortOrder;

    resort();
    emit sortOrderChanged(newSortOrder);
    emit isRowOrderSetChanged();
}

void NodeAttributeTableDisplayModel::reset()
{
    beginResetModel();

    // Remove any sorting criteria that might not exist any more
    _sortColumnAndOrders.erase(std::ranges::remove_if(_sortColumnAndOrders,
    [this](const auto& sortColumnAndOrder)
    {
        return _columnNames.indexOf(sortColumnAndOrder.first) < 0;
    }).begin(), _sortColumnAndOrders.end());

    updateSourceColumns();
    auto rowsChanged = updateSourceRows();

    endResetModel();

    emit columnOrderChanged();

    if(rowsChanged)
        emit sourceRowsChanged();
}

void NodeAttributeTableDisplayModel::updateSourceColumns()
{
    Q_ASSERT(_sourceModel == nullptr || _columnOrder.size() == _sourceModel->columnCount());

    _sourceColumns.clear();

    for(const auto& columnName : std::as_const(_columnOrder))
    {
        if(u::contains(_hiddenColumns, columnName))
            continue;

        auto sourceColumn = _columnNames.indexOf(columnName);
        Q_ASSERT(sourceColumn >= 0);
        if(sourceColumn >= 0)
            _sourceColumns.emplace_back(static_cast<int>(sourceColumn));
    }

    _headerModel.clear();
    _headerModel.setRowCount(1);
    _headerModel.setColumnCount(columnCount());

    // Headermodel takes ownership of the Items
    for(int i = 0; i < columnCount(); ++i)
        _headerModel.setItem(0, i, new QStandardItem());
}

static bool variantLessThan(const QVariant& a, const QVariant& b)
{
    if(!a.isValid())
        return false;

    if(!b.isValid())
        return true;

    switch(static_cast<QMetaType::Type>(a.typeId()))
    {
    case QMetaType::Int:    return a.toInt() < b.toInt();
    case QMetaType::Double: return a.toDouble() < b.toDouble();
    default: break;
    }

    return a.toString().compare(b.toString(), Qt::CaseSensitive) < 0;
}

void NodeAttributeTableDisplayModel::sortSourceRows()
{
    // The values to sort on, per sort column, per row in _sourceRows
    std::vector<std::pair<std::vector<QVariant>, Qt::SortOrder>> sortValues;

    for(const auto& [columnName, order] : _sortColumnAndOrders)
    {
        auto column = _columnNames.indexOf(columnName);

        Q_ASSERT(column >= 0);
        if(column < 0)
            continue;

        auto& values = sortValues.emplace_back(std::vector<QVariant>{}, order).first;
        values.reserve(_sourceRows.size());

        for(auto sourceRow : _sourceRows)
            values.emplace_back(_sourceModel->data(_sourceModel->index(sourceRow, static_cast<int>(column)), Qt::DisplayRole));
    }

    if(_rowOrderMap.empty() && sortValues.empty())
        return;

    auto lessThan = [&](size_t a, size_t b)
    {
        auto rowOrderA = _rowOrderMap.find(_sourceRows[a]);
        auto rowOrderB = _rowOrderMap.find(_sourceRows[b]);

        auto rowOrderMapContainsA = rowOrderA != _rowOrderMap.end();
        auto rowOrderMapContainsB = rowOrderB != _rowOrderMap.end();

        if(rowOrderMapContainsA && rowOrderMapContainsB)
            return rowOrderA->second < rowOrderB->second;

        if(rowOrderMapContainsA)
            return true;

        if(rowOrderMapContainsB)
            return false;

        for(const auto& [values, order] : sortValues)
        {
            const auto& valueA = values[a];
            const auto& valueB = values[b];

            if(valueA == valueB)
                continue;

            // Rows with no value go last, whichever the order
            if(!valueA.isValid() || !valueB.isValid())
                return valueA.isValid();

            if(static_cast<QMetaType::Type>(valueA.typeId()) == QMetaType::QString &&
                static_cast<QMetaType::Type>(valueB.typeId()) == QMetaType::QString)
            {
                return order == Qt::DescendingOrder ?
                    u::numericCompare(valueB.toString(), valueA.toString()) < 0 :
                    u::numericCompare(valueA.toString(), valueB.toString()) < 0;
            }

            return order == Qt::DescendingOrder ?
                variantLessThan(valueB, valueA) :
                variantLessThan(valueA, valueB);
        }

        return false;
    };

    // _sourceRows is in ascending order, which stable_sort retains where rows compare equal
    std::vector<size_t> order(_sourceRows.size());
    u::iota(order, 0);
    std::ranges::stable_sort(order, lessThan);

    std::vector<int> sortedSourceRows;
    sortedSourceRows.reserve(_sourceRows.size());

    for(auto i : order)
        sortedSourceRows.push_back(_sourceRows[i]);

    _sourceRows = std::move(sortedSourceRows);
}

// Returns whether the rows, or their order, changed
bool NodeAttributeTableDisplayModel::updateSourceRows()
{
    // The selection carries over to the new rows by data row, as all of them if every
    // row was selected, and as none if none were
    auto allSelected = std::ranges::find(_rowSelected, false) == _rowSelected.end();

    std::vector<int> selectedSourceRows;
    if(!allSelected)
    {
        for(size_t row = 0; row < _rowSelected.size(); row++)
        {
            if(_rowSelected.at(row))
                selectedSourceRows.push_back(_sourceRows.at(row));
        }
    }

    auto previousSourceRows = std::move(_sourceRows);
    _sourceRows.clear();
    _rowForSourceRow.clear();

    _rowSelected.clear();

    // The new selection is only published once whatever reacts to the new rows
    // has had the chance to select from them itself
    QMetaObject::invokeMethod(this, &NodeAttributeTableDisplayModel::publishSelection, Qt::QueuedConnection);

    if(_sourceModel == nullptr)
        return !previousSourceRows.empty();

    const auto numSourceRows = _sourceModel->rowCount();

    for(int sourceRow = 0; sourceRow < numSourceRows; sourceRow++)
    {
        if(_sourceModel->data(_sourceModel->index(sourceRow, 0), NodeAttributeTableModel::Roles::NodeSelectedRole).toBool())
            _sourceRows.push_back(sourceRow);
    }

    sortSourceRows();

    _rowForSourceRow.resize(static_cast<size_t>(numSourceRows), -1);

    for(size_t row = 0; row < _sourceRows.size(); row++)
        _rowForSourceRow[static_cast<size_t>(_sourceRows[row])] = static_cast<int>(row);

    _rowSelected.assign(_sourceRows.size(), allSelected);

    for(auto sourceRow : selectedSourceRows)
    {
        if(static_cast<size_t>(sourceRow) >= _rowForSourceRow.size())
            continue;

        auto row = _rowForSourceRow.at(static_cast<size_t>(sourceRow));
        if(row >= 0)
            _rowSelected.at(static_cast<size_t>(row)) = true;
    }

    return _sourceRows != previousSourceRows;
}

void NodeAttributeTableDisplayModel::resort()
{
    beginResetModel();
    auto rowsChanged = updateSourceRows();
    endResetModel();

    if(rowsChanged)
        emit sourceRowsChanged();
}

void NodeAttributeTableDisplayModel::onSourceSelectionChanged()
{
    beginResetModel();
    updateSourceRows();

    // A new graph selection starts with every row selected
    _rowSelected.assign(_sourceRows.size(), true);

    endResetModel();

    // Even the same rows count as new, so that whatever selects from them can do so again
    emit sourceRowsChanged();
    emit rowsReplaced();
}

void NodeAttributeTableDisplayModel::onSourceDataChanged(const QModelIndex& topLeft,
    const QModelIndex& bottomRight, const QList<int>& roles)
{
    // Any data row can be in any row here, so the change is taken to be to whole columns
    auto firstSourceColumn = topLeft.column();
    auto lastSourceColumn = bottomRight.column();

    auto sortColumnChanged = std::ranges::any_of(_sortColumnAndOrders,
    [&](const auto& sortColumnAndOrder)
    {
        auto sourceColumn = _columnNames.indexOf(sortColumnAndOrder.first);
        return sourceColumn >= firstSourceColumn && sourceColumn <= lastSourceColumn;
    });

    if(sortColumnChanged)
    {
        resort();
        return;
    }

    if(rowCount() == 0)
        return;

    for(int column = 0; column < columnCount(); column++)
    {
        auto sourceColumn = sourceColumnFor(column);
        if(sourceColumn >= firstSourceColumn && sourceColumn <= lastSourceColumn)
            emit dataChanged(index(0, column), index(rowCount() - 1, column), roles);
    }
}

// The source's column names only change when it resets
void NodeAttributeTableDisplayModel::onSourceModelReset()
{
    _columnNames.clear();

    if(_sourceModel != nullptr)
    {
        for(int column = 0; column < _sourceModel->columnCount(); column++)
            _columnNames.append(_sourceModel->columnNameFor(static_cast<size_t>(column)));
    }

    // Remove everything from _hiddenColumns that's not in _columnNames
    _hiddenColumns = u::toQStringList(u::setIntersection(
        static_cast<const QList<QString>&>(_hiddenColumns),
        static_cast<const QList<QString>&>(_columnNames)));

    setColumnOrder(_columnOrder);

    reset();
}

bool NodeAttributeTableDisplayModel::isRowOrderSet() const
{
    return !_rowOrderMap.empty();
}
