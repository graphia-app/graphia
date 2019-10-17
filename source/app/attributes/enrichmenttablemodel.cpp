#include "enrichmenttablemodel.h"
#include "enrichmentcalculator.h"

EnrichmentTableModel::EnrichmentTableModel(QObject *parent)
{
    setParent(parent);

    _roleNames[Qt::UserRole + Results::SelectionA] = "SelectionA";
    _roleNames[Qt::UserRole + Results::SelectionB] = "SelectionB";
    _roleNames[Qt::UserRole + Results::Observed] = "Observed";
    _roleNames[Qt::UserRole + Results::ExpectedTrial] = "ExpectedTrial";
    _roleNames[Qt::UserRole + Results::OverRep] = "OverRep";
    _roleNames[Qt::UserRole + Results::Fishers] = "Fishers";
    _roleNames[Qt::UserRole + Results::AdjustedFishers] = "AdjustedFishers";
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
    auto row = static_cast<size_t>(index.row());
    auto column = static_cast<size_t>(role - Qt::UserRole);

    Q_ASSERT(row < static_cast<size_t>(rowCount()));
    Q_ASSERT(column < static_cast<size_t>(columnCount()));

    const auto& dataRow = _data.at(row);
    auto value = dataRow.at(column);

    return value;
}

QVariant EnrichmentTableModel::data(int row, Results result) const
{
    return data(index(row, 0), result + Qt::UserRole);
}

int EnrichmentTableModel::rowFromAttributeSets(const QString& attributeA, const QString& attributeB)
{
    for(int rowIndex = 0; rowIndex < static_cast<int>(_data.size()); ++rowIndex)
    {
        if(data(rowIndex, Results::SelectionA) == attributeA &&
            data(rowIndex, Results::SelectionB) == attributeB)
        {
            return rowIndex;
        }
    }
    return -1;
}

json EnrichmentTableModel::toJson()
{
    json object;
    for(int rowIndex = 0; rowIndex < static_cast<int>(_data.size()); ++rowIndex)
    {
        for(int columnIndex = 0; columnIndex < static_cast<int>(_data[rowIndex].size()); ++columnIndex)
        {
            auto& variant = _data[rowIndex][columnIndex];
            if(variant.type() == QVariant::String)
                object["data"][rowIndex].push_back(_data[rowIndex][columnIndex].toString().toStdString());
            else if(variant.type() == QVariant::Double || variant.type() == QVariant::Int)
                object["data"][rowIndex].push_back(_data[rowIndex][columnIndex].toDouble());
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

QString EnrichmentTableModel::resultToString(Results result)
{
    switch(result)
    {
        case Results::SelectionA:
            return QStringLiteral("SelectionA");
        case Results::SelectionB:
            return QStringLiteral("SelectionB");
        case Results::Observed:
            return QStringLiteral("Observed");
        case Results::ExpectedTrial:
            return QStringLiteral("ExpectedTrial");
        case Results::OverRep:
            return QStringLiteral("OverRep");
        case Results::Fishers:
            return QStringLiteral("Fishers");
        case Results::AdjustedFishers:
            return QStringLiteral("AdjustedFishers");
        default:
            qDebug() << "Unknown roleEnum passed to resultToString";
        return {};
    }
}

bool EnrichmentTableModel::resultIsNumerical(EnrichmentTableModel::Results result)
{
    if(_data.empty())
        return false;

    const auto& firstRow = _data.at(0);
    if(result > firstRow.size())
        return false;

    auto variantType = firstRow.at(result).type();

    return variantType == QVariant::Double || variantType == QVariant::Int;
}
