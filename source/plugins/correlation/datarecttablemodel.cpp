#include "datarecttablemodel.h"
#include <QDebug>

bool DataRectTableModel::transposed() const
{
    return _transposed;
}

void DataRectTableModel::setTransposed(bool transposed)
{
    if(_transposed != transposed)
    {
        beginResetModel();
        _transposed = transposed;
        _data.setTransposed(_transposed);
        endResetModel();
    }
}

int DataRectTableModel::rowCount(const QModelIndex&) const
{
    return static_cast<int>(_data.numRows());
}

int DataRectTableModel::columnCount(const QModelIndex&) const
{
    return static_cast<int>(_data.numColumns());
}

QVariant DataRectTableModel::data(const QModelIndex& index, int role) const
{
    if(role < Qt::UserRole)
        return {};
    size_t row = index.row();
    size_t column = (role - Qt::UserRole);

    auto& value = _data.valueAt(column, row);

    return QString::fromStdString(value);
}

QHash<int, QByteArray> DataRectTableModel::roleNames() const
{
    QHash<int, QByteArray> _roleNames;
    for(int i = 0; i < static_cast<int>(_data.numColumns()); ++i)
        _roleNames.insert(Qt::UserRole + i, QByteArray::number(i));

    return _roleNames;
}

void DataRectTableModel::setTabularData(TabularData& data)
{
    beginResetModel();
    _data = std::move(data);
    _data.setTransposed(_transposed);
    endResetModel();
}

TabularData* DataRectTableModel::tabularData()
{
    return &_data;
}
