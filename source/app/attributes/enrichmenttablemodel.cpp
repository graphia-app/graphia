#include "enrichmenttablemodel.h"
#include "enrichmentcalculator.h"

EnrichmentTableModel::EnrichmentTableModel(QObject *parent)
{
    setParent(parent);
}

int EnrichmentTableModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return static_cast<int>(_data.size());
}

int EnrichmentTableModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return Results::NumResultColumns;
}

QVariant EnrichmentTableModel::data(const QModelIndex& index, int role) const
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
    _roleNames[Qt::UserRole + Results::SelectionA] = "SelectionA";
    _roleNames[Qt::UserRole + Results::SelectionB] = "SelectionB";
    _roleNames[Qt::UserRole + Results::Observed] = "Observed";
    _roleNames[Qt::UserRole + Results::Expected] = "Expected";
    _roleNames[Qt::UserRole + Results::ExpectedTrial] = "ExpectedTrial";
    _roleNames[Qt::UserRole + Results::OverRep] = "OverRep";
    _roleNames[Qt::UserRole + Results::Fishers] = "Fishers";
    return _roleNames;
}

json EnrichmentTableModel::toJson()
{
    json object;
    for(int rowIndex = 0; rowIndex < static_cast<int>(_data.size()); ++rowIndex)
    {
        for(int columnIndex = 0; columnIndex < static_cast<int>(_data[rowIndex].size()); ++columnIndex)
        {
            object["data"][rowIndex].push_back(_data[rowIndex][columnIndex].toString().toStdString());
        }
    }
    return object;
}

void EnrichmentTableModel::setTableData(EnrichmentTableModel::Table data)
{
    beginResetModel();
    _data = std::move(data);
    endResetModel();
}
