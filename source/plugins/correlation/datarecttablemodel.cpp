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
        if(_data != nullptr)
            _data->setTransposed(_transposed);
        endResetModel();
    }
}

int DataRectTableModel::rowCount(const QModelIndex&) const
{
    if(_data != nullptr)
        return static_cast<int>(_data->numRows());
    return 0;
}

int DataRectTableModel::columnCount(const QModelIndex&) const
{
    if(_data != nullptr)
        return static_cast<int>(_data->numColumns());
    return 0;
}

QVariant DataRectTableModel::data(const QModelIndex& index, int role) const
{
    Q_UNUSED(role)

    auto row = static_cast<size_t>(index.row());
    auto column = static_cast<size_t>(index.column());

    if(row >= _data->numRows() || column >= _data->numColumns())
        return  {};

    return _data->valueAt(column, row);
}

QHash<int, QByteArray> DataRectTableModel::roleNames() const
{
    return { {Qt::DisplayRole, "display"} };
}

void DataRectTableModel::setTabularData(TabularData& data)
{
    beginResetModel();
    _data = &data;
    _data->setTransposed(_transposed);
    endResetModel();
}
