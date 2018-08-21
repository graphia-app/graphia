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
    if(role < Qt::UserRole)
        return {};
    size_t row = index.row();
    size_t column = (role - Qt::UserRole);

    if(row >= _data->numRows() || column >= _data->numColumns())
        return  {};
    auto& value = _data->valueAt(column, row);

    return QString::fromStdString(value);
}

QHash<int, QByteArray> DataRectTableModel::roleNames() const
{
    QHash<int, QByteArray> roleNames;
    // FIXME: Have to hard limit rolenames or else tableview will crash.
    // https://bugreports.qt.io/browse/QTBUG-70069
    for(int i = 0; i < std::min(MAX_COLUMNS, static_cast<int>(_data->numColumns())); ++i)
        roleNames.insert(Qt::UserRole + i, QByteArray::number(i));

    return roleNames;
}

void DataRectTableModel::setTabularData(TabularData& data)
{
    beginResetModel();
    _data = &data;
    _data->setTransposed(_transposed);
    endResetModel();
}
