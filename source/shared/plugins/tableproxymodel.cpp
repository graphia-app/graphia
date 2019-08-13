#include "tableproxymodel.h"

#include "nodeattributetablemodel.h"

bool TableProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    return sourceModel()->data(sourceModel()->index(sourceRow, 0, sourceParent),
                               NodeAttributeTableModel::Roles::NodeSelectedRole).toBool();
}

QVariant TableProxyModel::data(const QModelIndex &index, int role) const
{
    if(role == SubSelectedRole)
    {
        return _subSelection.contains(index);
    }
    return sourceModel()->data(index, role);
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

int TableProxyModel::mapToSource(int proxyRow) const
{
    QModelIndex proxyIndex = index(proxyRow, 0);
    QModelIndex sourceIndex = mapToSource(proxyIndex);
    return sourceIndex.isValid() ? sourceIndex.row() : -1;
}

TableProxyModel::TableProxyModel(QObject *parent) : QSortFilterProxyModel (parent)
{
    connect(this, &QAbstractItemModel::rowsInserted, this, &TableProxyModel::countChanged);
    connect(this, &QAbstractItemModel::rowsRemoved, this, &TableProxyModel::countChanged);
    connect(this, &QAbstractItemModel::modelReset, this, &TableProxyModel::countChanged);
    connect(this, &QAbstractItemModel::layoutChanged, this, &TableProxyModel::countChanged);
}

void TableProxyModel::invalidateFilter()
{
    QSortFilterProxyModel::invalidateFilter();
}
