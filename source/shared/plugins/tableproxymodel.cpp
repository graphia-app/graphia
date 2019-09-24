#include "tableproxymodel.h"

#include "nodeattributetablemodel.h"

bool TableProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    return sourceModel()->data(sourceModel()->index(sourceRow, 0, sourceParent),
                               NodeAttributeTableModel::Roles::NodeSelectedRole).toBool();
}

bool TableProxyModel::filterAcceptsColumn(int sourceColumn, const QModelIndex &sourceParent) const
{
    return !u::contains(_hiddenColumns, sourceColumn);
}

QVariant TableProxyModel::data(const QModelIndex &index, int role) const
{
    if(role == SubSelectedRole)
    {
        return _subSelection.contains(index);
    }

    auto unorderedSourceIndex = mapToSource(index);

    if(_mappedColumnOrder.size() == columnCount())
    {
        auto mappedIndex = sourceModel()->index(unorderedSourceIndex.row(), _mappedColumnOrder.at(index.column()));
        return sourceModel()->data(mappedIndex, role);
    }
    else
    {
        auto sourceIndex = sourceModel()->index(unorderedSourceIndex.row(), unorderedSourceIndex.column());
        if (index.isValid() && !sourceIndex.isValid())
            return {};
        return sourceModel()->data(sourceIndex, role);
    }
}

void TableProxyModel::setSubSelection(QModelIndexList subSelection)
{
    _subSelection = subSelection;
    QList<QPersistentModelIndex> persistentModelIndexList;
    for(auto modelIndex : subSelection)
        persistentModelIndexList.push_back(modelIndex);
    emit layoutChanged(persistentModelIndexList);
}

QItemSelectionRange TableProxyModel::buildRowSelectionRange(int topRow, int bottomRow)
{
    return QItemSelectionRange(index(topRow, 0), index(bottomRow, columnCount() - 1));
}

int TableProxyModel::mapToSourceRow(int proxyRow) const
{
    QModelIndex proxyIndex = index(proxyRow, 0);
    QModelIndex sourceIndex = mapToSource(proxyIndex);
    return sourceIndex.isValid() ? sourceIndex.row() : -1;
}

int TableProxyModel::mapToSourceColumn(int proxyColumn) const
{
    if(proxyColumn >= columnCount())
        return -1;

    auto mappedProxyColumn = proxyColumn;
    if(_mappedColumnOrder.size() > 0)
        mappedProxyColumn = _mappedColumnOrder.at(proxyColumn);

    return mappedProxyColumn;
}

TableProxyModel::TableProxyModel(QObject *parent) : QSortFilterProxyModel (parent)
{
    connect(this, &QAbstractItemModel::rowsInserted, this, &TableProxyModel::countChanged);
    connect(this, &QAbstractItemModel::rowsRemoved, this, &TableProxyModel::countChanged);
    connect(this, &QAbstractItemModel::modelReset, this, &TableProxyModel::countChanged);
    connect(this, &QAbstractItemModel::layoutChanged, this, &TableProxyModel::countChanged);
}

void TableProxyModel::setHiddenColumns(std::vector<int> hiddenColumns)
{
    std::sort(hiddenColumns.begin(), hiddenColumns.end());
    _hiddenColumns = hiddenColumns;
    invalidateFilter();
    recalculateOrderMapping();
}

void TableProxyModel::recalculateOrderMapping()
{
    if(_sourceColumnOrder.size() != sourceModel()->columnCount())
    {
        // If ordering doesn't match the sourcemodel size just destroy it
        _sourceColumnOrder = std::vector<int>(sourceModel()->columnCount());
        std::iota(_sourceColumnOrder.begin(), _sourceColumnOrder.end(), 0);
    }

    auto filteredOrder = u::setDifference(_sourceColumnOrder, _hiddenColumns);
    _mappedColumnOrder = filteredOrder;

    emit columnOrderChanged();
    emit layoutChanged();
}

void TableProxyModel::setColumnOrder(std::vector<int> columnOrder)
{
    _sourceColumnOrder = columnOrder;

    recalculateOrderMapping();
    invalidateFilter();
}

void TableProxyModel::setSortColumn(int sortColumn)
{
    _sortColumn = sortColumn;

    auto mappedColumn = mapFromSource(sourceModel()->index(0, _sortColumn));

    this->sort(mappedColumn.column(), _sortOrder);
    emit sortColumnChanged(sortColumn);
}

void TableProxyModel::setSortOrder(Qt::SortOrder sortOrder)
{
    _sortOrder = sortOrder;

    auto mappedColumn = mapFromSource(sourceModel()->index(0, _sortColumn));

    this->sort(mappedColumn.column(), _sortOrder);
    emit sortOrderChanged(sortOrder);
}

void TableProxyModel::invalidateFilter()
{
    QSortFilterProxyModel::invalidateFilter();
    recalculateOrderMapping();
}
