#include "enrichmenttablemodel.h"
#include <QDebug>

EnrichmentTableModel::EnrichmentTableModel(QObject *parent)
{
    setParent(parent);
}

int EnrichmentTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return static_cast<int>(_data.size());
}

int EnrichmentTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return COLUMN_COUNT;
}

QVariant EnrichmentTableModel::data(const QModelIndex &index, int role) const
{
    if(role < Qt::UserRole)
        return {};
    size_t row = index.row();
    size_t column = (role - Qt::UserRole);

    const auto& dataRow = _data.at(row);
    auto value = dataRow.at(column);

    return value;
}

QVariant EnrichmentTableModel::data(int row, const QString& role)
{
    return data(index(row,0), roleNames().key(role.toUtf8()));
}

int EnrichmentTableModel::rowFromAttributeSets(const QString& attributeA, const QString& attributeB)
{
    for(int rowIndex = 0; rowIndex < static_cast<int>(_data.size()); ++rowIndex)
    {
        if(data(rowIndex, QStringLiteral("Attribute Group")) == attributeA
                && data(rowIndex, QStringLiteral("Selection")) == attributeB)
            return rowIndex;
    }
    return -1;
}

QHash<int, QByteArray> EnrichmentTableModel::roleNames() const
{
    QHash<int, QByteArray> _roleNames;
    _roleNames[Qt::UserRole] = "Attribute Group";
    _roleNames[Qt::UserRole + 1] = "Selection";
    _roleNames[Qt::UserRole + 2] = "Observed";
    _roleNames[Qt::UserRole + 3] = "Expected";
    _roleNames[Qt::UserRole + 4] = "ExpectedTrial";
    _roleNames[Qt::UserRole + 5] = "OverRep";
    _roleNames[Qt::UserRole + 6] = "Fishers";
    return _roleNames;
}

json EnrichmentTableModel::toJson()
{
    json object;
    // For each added rolename
    for (int i = 0; i < COLUMN_COUNT; ++i)
        object["rolenames"][i] = roleNames()[Qt::UserRole + i].toStdString();

    for(int rowIndex = 0; rowIndex < static_cast<int>(_data.size()); ++rowIndex)
    {
        for(int columnIndex = 0; columnIndex < static_cast<int>(_data[rowIndex].size()); ++columnIndex)
        {
            object["data"][rowIndex].push_back(_data[rowIndex][columnIndex].toString().toStdString());
        }
    }
    qDebug() << QString::fromStdString(object.dump());
    return object;
}

void EnrichmentTableModel::setTableData(EnrichmentTableModel::Table data)
{
    beginResetModel();
    _data = std::move(data);
    endResetModel();
}
