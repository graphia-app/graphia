#include "correlationnodeattributetablemodel.h"

#include "shared/utils/container.h"

QStringList CorrelationNodeAttributeTableModel::columnNames() const
{
    auto list = NodeAttributeTableModel::columnNames();

    if(_dataColumnNames != nullptr)
    {
        list.reserve(list.size() + static_cast<int>(_dataColumnNames->size()));
        for(const auto& dataColumnName : *_dataColumnNames)
            list.append(dataColumnName);
    }

    return list;
}

QVariant CorrelationNodeAttributeTableModel::dataValue(size_t row, const QString& columnName) const
{
    if(!_dataColumnIndexes.empty() && u::contains(_dataColumnIndexes, columnName))
    {
        size_t column = _dataColumnIndexes.at(columnName);
        size_t index = (row * _dataColumnIndexes.size()) + column;

        Q_ASSERT(index < _dataValues->size());
        return _dataValues->at(index);
    }

    return NodeAttributeTableModel::dataValue(row, columnName);
}

void CorrelationNodeAttributeTableModel::addDataColumns(std::vector<QString>* dataColumnNames,
    std::vector<double>* dataValues)
{
    _dataColumnNames = dataColumnNames;
    _dataValues = dataValues;

    for(size_t i = 0; i < _dataColumnNames->size(); i++)
        _dataColumnIndexes.emplace(_dataColumnNames->at(i), i);

    // Check that the data column names are unique
    Q_ASSERT(_dataColumnNames->size() == _dataColumnIndexes.size());
}

bool CorrelationNodeAttributeTableModel::columnIsCalculated(const QString& columnName) const
{
    if(u::contains(_dataColumnIndexes, columnName))
        return false;

    return NodeAttributeTableModel::columnIsCalculated(columnName);
}

bool CorrelationNodeAttributeTableModel::columnIsHiddenByDefault(const QString& columnName) const
{
    if(u::contains(_dataColumnIndexes, columnName))
        return true;

    return NodeAttributeTableModel::columnIsHiddenByDefault(columnName);
}

bool CorrelationNodeAttributeTableModel::columnIsFloatingPoint(const QString& columnName) const
{
    if(u::contains(_dataColumnIndexes, columnName))
        return true; // All data columns are floating point

    return NodeAttributeTableModel::columnIsFloatingPoint(columnName);
}
