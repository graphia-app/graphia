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

#ifndef CORRELATIONNODEATTRIBUTETABLEMODEL_H
#define CORRELATIONNODEATTRIBUTETABLEMODEL_H

#include "shared/plugins/nodeattributetablemodel.h"

#include <QString>
#include <QObject>

#include <vector>
#include <map>

class CorrelationNodeAttributeTableModel : public NodeAttributeTableModel
{
    Q_OBJECT

private:
    std::vector<QString> _dataColumnNames;
    std::vector<double>* _continuousDataValues = nullptr;
    std::vector<QString>* _discreteDataValues = nullptr;

    // For fast lookup in dataValue(...)
    std::map<QString, size_t> _dataColumnIndexes;

    QStringList columnNames() const override;

    void addDataColumnNames(const std::vector<QString>& dataColumnNames);

public:
    void addContinuousDataColumns(const std::vector<QString>& dataColumnNames,
        std::vector<double>* dataValues);
    void addDiscreteDataColumns(const std::vector<QString>& dataColumnNames,
        std::vector<QString>* dataValues);

    QVariant dataValue(size_t row, const QString& columnName) const override;

    bool columnIsCalculated(const QString& columnName) const override;
    bool columnIsHiddenByDefault(const QString& columnName) const override;
    bool columnIsFloatingPoint(const QString& columnName) const override;
};

#endif // CORRELATIONNODEATTRIBUTETABLEMODEL_H
