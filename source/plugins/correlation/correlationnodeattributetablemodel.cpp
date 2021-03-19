/* Copyright © 2013-2020 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "correlationnodeattributetablemodel.h"

#include "shared/utils/container.h"

#include <algorithm>

QStringList CorrelationNodeAttributeTableModel::columnNames() const
{
    auto list = NodeAttributeTableModel::columnNames();

    list.reserve(list.size() + static_cast<int>(_dataColumnNames.size()));
    for(const auto& dataColumnName : _dataColumnNames)
        list.append(dataColumnName);

    return list;
}

void CorrelationNodeAttributeTableModel::addDataColumnNames(const std::vector<QString>& dataColumnNames)
{
    _dataColumnNames = dataColumnNames;

    // Since these columns are appearing in the attribute table, we need to assign them
    // names that don't conflict with attribute names
    std::transform(_dataColumnNames.begin(), _dataColumnNames.end(), _dataColumnNames.begin(),
    [](const auto& columnName)
    {
        return QStringLiteral("Data Value › %1").arg(columnName);
    });

    for(size_t i = 0; i < _dataColumnNames.size(); i++)
        _dataColumnIndexes.emplace(_dataColumnNames.at(i), i);

    // Check that the data column names are unique
    Q_ASSERT(_dataColumnNames.size() == _dataColumnIndexes.size());
}

QVariant CorrelationNodeAttributeTableModel::dataValue(size_t row, const QString& columnName) const
{
    if(!_dataColumnIndexes.empty() && u::contains(_dataColumnIndexes, columnName))
    {
        size_t column = _dataColumnIndexes.at(columnName);
        size_t index = (row * _dataColumnIndexes.size()) + column;

        if(_continuousDataValues != nullptr)
        {
            Q_ASSERT(index < _continuousDataValues->size());
            return _continuousDataValues->at(index);
        }

        if(_discreteDataValues != nullptr)
        {
            Q_ASSERT(index < _discreteDataValues->size());
            return _discreteDataValues->at(index);
        }

        return {};
    }

    return NodeAttributeTableModel::dataValue(row, columnName);
}

void CorrelationNodeAttributeTableModel::addContinuousDataColumns(const std::vector<QString>& dataColumnNames,
    std::vector<double>* dataValues)
{
    addDataColumnNames(dataColumnNames);
    Q_ASSERT(_discreteDataValues == nullptr);
    _continuousDataValues = dataValues;
}

void CorrelationNodeAttributeTableModel::addDiscreteDataColumns(const std::vector<QString>& dataColumnNames,
    std::vector<QString>* dataValues)
{
    addDataColumnNames(dataColumnNames);
    Q_ASSERT(_continuousDataValues == nullptr);
    _discreteDataValues = dataValues;
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
        return _continuousDataValues != nullptr;

    return NodeAttributeTableModel::columnIsFloatingPoint(columnName);
}
