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

QVariant CorrelationNodeAttributeTableModel::dataValue(int row, int role) const
{
    if(_dataColumnNames != nullptr && _data != nullptr &&
        _firstDataColumnRole > 0 && role >= _firstDataColumnRole)
    {
        size_t column = role - _firstDataColumnRole;
        size_t index = (row * _dataColumnNames->size()) + column;

        Q_ASSERT(index < _data->size());
        return _data->at(index);
    }

    return NodeAttributeTableModel::dataValue(row, role);
}

void CorrelationNodeAttributeTableModel::initialise(IDocument* document, UserNodeData* userNodeData,
                                                    std::vector<QString>* dataColumnNames,
                                                    std::vector<double>* data)
{
    _dataColumnNames = dataColumnNames;
    _data = data;

    NodeAttributeTableModel::initialise(document, userNodeData);
}

void CorrelationNodeAttributeTableModel::updateRoleNames()
{
    NodeAttributeTableModel::updateRoleNames();

    _firstDataColumnRole = -1;
    if(_dataColumnNames != nullptr && !_dataColumnNames->empty())
        _firstDataColumnRole = roleNames().key(_dataColumnNames->front().toUtf8());
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
