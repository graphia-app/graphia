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

QVariant CorrelationNodeAttributeTableModel::dataValue(size_t row, int attributeIndex) const
{
    if(_dataColumnNames != nullptr && _dataValues != nullptr &&
        _firstDataColumnRole > 0 && attributeIndex >= _firstDataColumnRole)
    {
        size_t column = static_cast<size_t>(attributeIndex) - static_cast<size_t>(_firstDataColumnRole);
        size_t index = (row * _dataColumnNames->size()) + column;

        Q_ASSERT(index < _dataValues->size());
        return _dataValues->at(index);
    }

    return NodeAttributeTableModel::dataValue(row, attributeIndex);
}

void CorrelationNodeAttributeTableModel::initialise(IDocument* document, UserNodeData* userNodeData,
                                                    std::vector<QString>* dataColumnNames,
                                                    std::vector<double>* dataValues)
{
    _dataColumnNames = dataColumnNames;
    _dataValues = dataValues;

    NodeAttributeTableModel::initialise(document, userNodeData);
}

void CorrelationNodeAttributeTableModel::updateColumnNames()
{
    NodeAttributeTableModel::updateColumnNames();

    if(_dataColumnNames != nullptr && !_dataColumnNames->empty())
    {
        auto attributeCount = NodeAttributeTableModel::columnNames().size();
        _firstDataColumnRole = attributeCount;
    }
}

bool CorrelationNodeAttributeTableModel::columnIsCalculated(const QString& columnName) const
{
    if(_dataColumnNames != nullptr && u::contains(*_dataColumnNames, columnName))
        return false;

    return NodeAttributeTableModel::columnIsCalculated(columnName);
}

bool CorrelationNodeAttributeTableModel::columnIsHiddenByDefault(const QString& columnName) const
{
    if(_dataColumnNames != nullptr && u::contains(*_dataColumnNames, columnName))
        return true;

    return NodeAttributeTableModel::columnIsHiddenByDefault(columnName);
}

bool CorrelationNodeAttributeTableModel::columnIsFloatingPoint(const QString& columnName) const
{
    if(_dataColumnNames != nullptr && u::contains(*_dataColumnNames, columnName))
        return true;

    return NodeAttributeTableModel::columnIsFloatingPoint(columnName);
}
