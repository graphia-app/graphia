#include "tableproxymodel.h"
#include "nodeattributetablemodel.h"

bool TableProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    return sourceModel()->data(sourceModel()->index(sourceRow, 0, sourceParent),
                               NodeAttributeTableModel::Roles::NodeSelectedRole).toBool();
}

bool TableProxyModel::filterAcceptsColumn(int sourceColumn, const QModelIndex &sourceParent) const
{
    Q_UNUSED(sourceParent)
    return !u::contains(_hiddenColumns, sourceColumn);
}

QVariant TableProxyModel::data(const QModelIndex &index, int role) const
{
    if(role == SubSelectedRole)
        return _subSelectionRows.find(index.row()) != _subSelectionRows.end();

    auto unorderedSourceIndex = mapToSource(index);

    if(_mappedColumnOrder.size() == static_cast<size_t>(columnCount()))
    {
        auto mappedIndex = sourceModel()->index(unorderedSourceIndex.row(), _mappedColumnOrder.at(index.column()));
        return sourceModel()->data(mappedIndex, role);
    }

    auto sourceIndex = sourceModel()->index(unorderedSourceIndex.row(), unorderedSourceIndex.column());
    if (index.isValid() && !sourceIndex.isValid())
    {
        qDebug() << "Invalid Source Index. Row: " << index.row() << "Col:" << index.column();
        return {};
    }
    return sourceModel()->data(sourceIndex, role);
}

void TableProxyModel::setSubSelection(const QItemSelection& subSelection, const QItemSelection& subDeSelection)
{
    _subSelection = subSelection;

    // Group selection by rows, no need to keep track of the indices for the model other than
    // emitting signals.
    _subSelectionRows.clear();
    for(auto index : _subSelection.indexes())
        _subSelectionRows.insert(index.row());

    for(const auto& range : _subSelection)
        emit dataChanged(range.topLeft(), range.bottomRight(), { Roles::SubSelectedRole });
    for(const auto& range : subDeSelection)
        emit dataChanged(range.topLeft(), range.bottomRight(), { Roles::SubSelectedRole });
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
    if(!_mappedColumnOrder.empty())
        mappedProxyColumn = _mappedColumnOrder.at(static_cast<size_t>(proxyColumn));

    return mappedProxyColumn;
}

TableProxyModel::TableProxyModel(QObject *parent) : QSortFilterProxyModel (parent)
{
    connect(this, &QAbstractProxyModel::sourceModelChanged, this, &TableProxyModel::invalidateFilter);
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
    if(_sourceColumnOrder.size() != static_cast<size_t>(sourceModel()->columnCount()))
    {
        // If ordering doesn't match the sourcemodel size just destroy it
        _sourceColumnOrder = std::vector<int>(static_cast<size_t>(sourceModel()->columnCount()));
        std::iota(_sourceColumnOrder.begin(), _sourceColumnOrder.end(), 0);
    }

    auto filteredOrder = u::setDifference(_sourceColumnOrder, _hiddenColumns);
    _mappedColumnOrder = filteredOrder;

    _headerModel.clear();
    _headerModel.setRowCount(1);
    _headerModel.setColumnCount(columnCount());
    for(int i = 0; i < columnCount(); ++i)
        _headerModel.setItem(0, i, new QStandardItem());

    emit columnOrderChanged();
    emit layoutChanged();
}

void TableProxyModel::setColumnOrder(const std::vector<int>& columnOrder)
{
    _sourceColumnOrder = columnOrder;
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
