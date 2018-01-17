#include "enrichmenttablemodel.h"
#include <QDebug>

EnrichmentTableModel::EnrichmentTableModel()
{
}

int EnrichmentTableModel::rowCount(const QModelIndex &parent) const
{
    return _data.size();
}

int EnrichmentTableModel::columnCount(const QModelIndex &parent) const
{
    return COLUMN_COUNT;
}

QVariant EnrichmentTableModel::data(const QModelIndex &index, int role) const
{
    size_t row = index.row();
    size_t column = (role - Qt::UserRole);

    const auto& dataRow = _data.at(row);
    auto value = dataRow.at(column);

    qDebug() << "DATA ACCESS";
    qDebug() << value;
    return value;
}

QHash<int, QByteArray> EnrichmentTableModel::roleNames() const
{
    QHash<int, QByteArray> _roleNames;
    _roleNames[Qt::UserRole] = "Selection";
    _roleNames[Qt::UserRole + 1] = "Observed";
    _roleNames[Qt::UserRole + 2] = "Expected";
    _roleNames[Qt::UserRole + 3] = "ExpectedTrial";
    _roleNames[Qt::UserRole + 4] = "FObs";
    _roleNames[Qt::UserRole + 5] = "FExp";
    _roleNames[Qt::UserRole + 6] = "OverRep";
    _roleNames[Qt::UserRole + 7] = "ZScore";
    _roleNames[Qt::UserRole + 8] = "Fishers";
    return _roleNames;
}

void EnrichmentTableModel::updateComplete()
{
    beginResetModel();
    endResetModel();
}

void EnrichmentTableModel::setTableData(EnrichmentTableModel::Table data)
{
    beginResetModel();
    _data = data;
    endResetModel();
    QMetaObject::invokeMethod(this, "updateComplete");
    emit dataChanged(createIndex(0,0), createIndex(_data.size(), COLUMN_COUNT));

}
