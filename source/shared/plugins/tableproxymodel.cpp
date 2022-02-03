/* Copyright © 2013-2022 Graphia Technologies Ltd.
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

#include "tableproxymodel.h"

#include "nodeattributetablemodel.h"

#include "shared/utils/container.h"
#include "shared/utils/string.h"
#include "shared/utils/static_block.h"

#include <QQmlEngine>

bool TableProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    return sourceModel()->data(sourceModel()->index(sourceRow, 0, sourceParent),
        NodeAttributeTableModel::Roles::NodeSelectedRole).toBool();
}

bool TableProxyModel::filterAcceptsColumn(int sourceColumn, const QModelIndex&) const
{
    auto columnName = _columnNames.at(sourceColumn);
    return !u::contains(_hiddenColumns, columnName);
}

QVariant TableProxyModel::data(const QModelIndex& index, int role) const
{
    if(role == SubSelectedRole)
        return u::contains(_subSelectionRows, index.row());

    auto unorderedSourceIndex = mapToSource(index);

    if(_orderedProxyToSourceColumn.size() == static_cast<size_t>(columnCount()))
    {
        auto mappedIndex = sourceModel()->index(unorderedSourceIndex.row(),
            _orderedProxyToSourceColumn.at(index.column()));
        return sourceModel()->data(mappedIndex, role);
    }

    auto sourceIndex = sourceModel()->index(unorderedSourceIndex.row(), unorderedSourceIndex.column());
    Q_ASSERT(index.isValid() && sourceIndex.isValid());

    return sourceModel()->data(sourceIndex, role);
}

void TableProxyModel::setSubSelection(const QItemSelection& subSelection, const QItemSelection& subDeSelection)
{
    _subSelection = subSelection;

    // Group selection by rows, no need to keep track of the indices for the model other than
    // emitting signals.
    _subSelectionRows.clear();

    const auto& indexes = _subSelection.indexes();
    for(auto index : indexes)
        _subSelectionRows.insert(index.row());

    for(const auto& range : std::as_const(_subSelection))
        emit dataChanged(range.topLeft(), range.bottomRight(), { Roles::SubSelectedRole });

    for(const auto& range : subDeSelection)
        emit dataChanged(range.topLeft(), range.bottomRight(), { Roles::SubSelectedRole });
}

QItemSelectionRange TableProxyModel::buildRowSelectionRange(int topRow, int bottomRow)
{
    return {index(topRow, 0), index(bottomRow, columnCount() - 1)};
}

int TableProxyModel::mapToSourceRow(int proxyRow) const
{
    QModelIndex proxyIndex = index(proxyRow, 0);
    QModelIndex sourceIndex = mapToSource(proxyIndex);
    return sourceIndex.isValid() ? sourceIndex.row() : -1;
}

int TableProxyModel::mapOrderedToSourceColumn(int proxyColumn) const
{
    if(proxyColumn >= columnCount())
        return -1;

    if(static_cast<int>(_orderedProxyToSourceColumn.size()) != columnCount())
        return -1;

    auto mappedProxyColumn = proxyColumn;
    if(!_orderedProxyToSourceColumn.empty())
        mappedProxyColumn = _orderedProxyToSourceColumn.at(static_cast<size_t>(proxyColumn));

    return mappedProxyColumn;
}

TableProxyModel::TableProxyModel(QObject* parent) : QSortFilterProxyModel(parent)
{
    connect(this, &QAbstractProxyModel::sourceModelChanged, this, &TableProxyModel::updateSourceModelFilter);
    connect(this, &TableProxyModel::columnNamesChanged, this, &TableProxyModel::onColumnNamesChanged);

    _collator.setNumericMode(true);
}

void TableProxyModel::setHiddenColumns(const QStringList& hiddenColumns)
{
    _hiddenColumns = hiddenColumns;
    invalidateFilter();
}

void TableProxyModel::calculateOrderedProxySourceMapping()
{
    Q_ASSERT(!sourceModel() || _sourceColumnOrder.size() == sourceModel()->columnCount());

    _orderedProxyToSourceColumn.clear();

    std::map<QString, int> columnNameIndicies;
    int index = 0;
    for(const auto& columnName : std::as_const(_columnNames))
        columnNameIndicies.emplace(columnName, index++);

    for(const auto& sourceColumnName : std::as_const(_sourceColumnOrder))
    {
        if(!u::contains(_hiddenColumns, sourceColumnName))
        {
            Q_ASSERT(u::contains(columnNameIndicies, sourceColumnName));
            _orderedProxyToSourceColumn.emplace_back(columnNameIndicies.at(sourceColumnName));
        }
    }

    _headerModel.clear();
    _headerModel.setRowCount(1);
    _headerModel.setColumnCount(columnCount());
    // Headermodel takes ownership of the Items
    for(int i = 0; i < columnCount(); ++i)
        _headerModel.setItem(0, i, new QStandardItem());

    emit columnOrderChanged();
    emit layoutChanged();
}

void TableProxyModel::setColumnOrder(const QStringList& columnOrder)
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

    if(newColumnOrder != _sourceColumnOrder)
    {
        _sourceColumnOrder = newColumnOrder;
        invalidateFilter();
        emit columnOrderChanged();
    }
}

QString TableProxyModel::sortColumn_() const
{
    if(_sortColumnAndOrders.empty())
        return {};

    return _sortColumnAndOrders.front().first;
}

void TableProxyModel::setSortColumn(const QString& newSortColumn)
{
    if(newSortColumn.isEmpty() || newSortColumn == sortColumn_())
        return;

    auto currentSortOrder = Qt::AscendingOrder;

    auto existing = std::find_if(_sortColumnAndOrders.begin(), _sortColumnAndOrders.end(),
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
}

Qt::SortOrder TableProxyModel::sortOrder_() const
{
    if(_sortColumnAndOrders.empty())
        return Qt::DescendingOrder;

    return _sortColumnAndOrders.front().second;
}

void TableProxyModel::setSortOrder(Qt::SortOrder newSortOrder)
{
    if(_sortColumnAndOrders.empty() || newSortOrder == sortOrder_())
        return;

    _sortColumnAndOrders.front().second = newSortOrder;

    resort();
    emit sortOrderChanged(newSortOrder);
}

void TableProxyModel::invalidateFilter()
{
    beginResetModel();

    // Remove any sorting criteria that might not exist any more
    _sortColumnAndOrders.erase(std::remove_if(_sortColumnAndOrders.begin(), _sortColumnAndOrders.end(),
    [this](const auto& sortColumnAndOrder)
    {
        return _columnNames.indexOf(sortColumnAndOrder.first) < 0;
    }), _sortColumnAndOrders.end());

    QSortFilterProxyModel::invalidate();
    QSortFilterProxyModel::invalidateFilter();

    calculateOrderedProxySourceMapping();
    endResetModel();
}

void TableProxyModel::updateSourceModelFilter()
{
    if(sourceModel() == nullptr)
        return;

    connect(sourceModel(), &QAbstractItemModel::modelReset, this, &TableProxyModel::invalidateFilter);
    connect(sourceModel(), &QAbstractItemModel::layoutChanged, this, &TableProxyModel::invalidateFilter);
}

void TableProxyModel::resort()
{
    invalidate();

    // The parameters to this don't really matter, because the actual ordering is determined
    // by the implementation of lessThan, in combination with the contents of _sortColumnAndOrders
    sort(0);
}

void TableProxyModel::onColumnNamesChanged()
{
    // Remove everything from _hiddenColumns that's not in _columnNames
    _hiddenColumns = u::toQStringList(u::setIntersection(
        static_cast<const QList<QString>&>(_hiddenColumns),
        static_cast<const QList<QString>&>(_columnNames)));

    // Refresh column order
    setColumnOrder(_sourceColumnOrder);

    invalidateFilter();
}

bool TableProxyModel::lessThan(const QModelIndex& a, const QModelIndex& b) const
{
    auto rowA = a.row();
    auto rowB = b.row();

    for(const auto& sortColumnAndOrder : _sortColumnAndOrders)
    {
        auto column = _columnNames.indexOf(sortColumnAndOrder.first);

        Q_ASSERT(column >= 0);
        if(column < 0)
            continue;

        auto order = sortColumnAndOrder.second;

        auto indexA = sourceModel()->index(rowA, column);
        auto indexB = sourceModel()->index(rowB, column);

        auto valueA = sourceModel()->data(indexA, Qt::DisplayRole);
        auto valueB = sourceModel()->data(indexB, Qt::DisplayRole);

        if(valueA == valueB)
            continue;

        if(static_cast<QMetaType::Type>(valueA.type()) == QMetaType::QString &&
            static_cast<QMetaType::Type>(valueB.type()) == QMetaType::QString)
        {
            return order == Qt::DescendingOrder ?
                _collator.compare(valueB.toString(), valueA.toString()) < 0 :
                _collator.compare(valueA.toString(), valueB.toString()) < 0;
        }

        return order == Qt::DescendingOrder ?
            QSortFilterProxyModel::lessThan(indexB, indexA) :
            QSortFilterProxyModel::lessThan(indexA, indexB);
    }

    return false;
}

static_block
{
    qmlRegisterType<TableProxyModel>(APP_URI, APP_MAJOR_VERSION, APP_MINOR_VERSION, "TableProxyModel");
}
