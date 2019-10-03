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

void CorrelationNodeAttributeTableModel::initialise(IDocument* document, UserNodeData* userNodeData,
                                                    std::vector<QString>* dataColumnNames,
                                                    std::vector<double>* dataValues)
{
    //FIXME: effectively disable the functionality this class provides for now, as it's causing
    // too many performance problems with TableView; revisit this when the new TableView is available
    _dataColumnNames = nullptr;//dataColumnNames;
    _dataValues = nullptr;//dataValues;

    NodeAttributeTableModel::initialise(document, userNodeData);
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
