#include "attributestablemodel.h"

#include "correlationplugin.h"

AttributesTableModel::AttributesTableModel(Attributes* attributes) :
    QAbstractTableModel(),
    _rowAttributes(attributes)
{}

void AttributesTableModel::initialise()
{
    int role = Qt::UserRole + 1;
    for(auto& rowAttribute : *_rowAttributes)
        _roleNames.insert(role++, rowAttribute.name().toUtf8());

    emit columnNamesChanged();
}

void AttributesTableModel::setSelectedRowIndexes(std::vector<int>&& selectedRowIndexes)
{
    emit layoutAboutToBeChanged();
    _selectedRowIndexes = selectedRowIndexes;
    emit layoutChanged();
}

QStringList AttributesTableModel::columnNames() const
{
    QStringList list;

    for(auto& rowAttribute : *_rowAttributes)
        list.append(rowAttribute.name());

    return list;
}

int AttributesTableModel::rowCount(const QModelIndex&) const
{
    return static_cast<int>(_selectedRowIndexes.size());
}

int AttributesTableModel::columnCount(const QModelIndex&) const
{
    return static_cast<int>(_rowAttributes->size());
}

QVariant AttributesTableModel::data(const QModelIndex& index, int role) const
{
    int row = index.row();
    if(row >= 0 && row < rowCount() && role >= Qt::UserRole)
    {
        int rowIndex = _selectedRowIndexes.at(index.row());
        return _rowAttributes->value(rowIndex, _roleNames[role]);
    }

    return QVariant();
}
