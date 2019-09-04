#include "tableproxymodel.h"

#include "nodeattributetablemodel.h"

bool TableProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    return sourceModel()->data(sourceModel()->index(sourceRow, 0, sourceParent),
                               NodeAttributeTableModel::Roles::NodeSelectedRole).toBool();
}

bool TableProxyModel::filterAcceptsColumn(int sourceColumn, const QModelIndex &sourceParent) const
{
    return !_hiddenColumns.contains(sourceColumn);
}

QVariant TableProxyModel::data(const QModelIndex &index, int role) const
{
    auto mappedIndex = index;
    if(_columnOrder.size() == columnCount())
        mappedIndex = this->index(index.row(), _columnOrder.at(index.column()));

    if(role == SubSelectedRole)
    {
        return _subSelection.contains(mappedIndex);
    }
    return QSortFilterProxyModel::data(mappedIndex, role);
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
    int mappedIndex = proxyColumn;
    if(_columnOrder.size() - 1 >= proxyColumn)
        mappedIndex = _columnOrder.at(proxyColumn);
    QModelIndex proxyIndex = index(0, mappedIndex);
    QModelIndex sourceIndex = mapToSource(proxyIndex);
    return sourceIndex.isValid() ? sourceIndex.column() : -1;
}

TableProxyModel::TableProxyModel(QObject *parent) : QSortFilterProxyModel (parent)
{
    connect(this, &QAbstractItemModel::rowsInserted, this, &TableProxyModel::countChanged);
    connect(this, &QAbstractItemModel::rowsRemoved, this, &TableProxyModel::countChanged);
    connect(this, &QAbstractItemModel::modelReset, this, &TableProxyModel::countChanged);
    connect(this, &QAbstractItemModel::layoutChanged, this, &TableProxyModel::countChanged);
}

void TableProxyModel::setHiddenColumns(QList<int> hiddenColumns)
{
    _hiddenColumns = hiddenColumns;
    invalidateFilter();
}

void TableProxyModel::setColumnOrder(QList<int> columnOrder)
{
    _columnOrder = columnOrder;
    emit columnOrderChanged();
    emit layoutChanged();
}

void TableProxyModel::setSortColumn(int sortColumn)
{
    _sortColumn = sortColumn;
    this->sort(_sortColumn, _sortOrder);
    emit sortColumnChanged(sortColumn);
}

void TableProxyModel::setSortOrder(Qt::SortOrder sortOrder)
{
    _sortOrder = sortOrder;
    this->sort(_sortColumn, _sortOrder);
    emit sortOrderChanged(sortOrder);
}

void TableProxyModel::invalidateFilter()
{
    QSortFilterProxyModel::invalidateFilter();
}
