/* Copyright © 2013-2021 Graphia Technologies Ltd.
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

#include "enrichmenttablemodel.h"
#include "enrichmentcalculator.h"

#include <QDebug>

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

void EnrichmentTableModel::setSelectionA(const QString& selectionA)
{
    if(selectionA != _selectionA)
    {
        _selectionA = selectionA;
        emit selectionNamesChanged();
    }
}

void EnrichmentTableModel::setSelectionB(const QString& selectionB)
{
    if(selectionB != _selectionB)
    {
        _selectionB = selectionB;
        emit selectionNamesChanged();
    }
}

void EnrichmentTableModel::setTableData(EnrichmentTableModel::Table data, QString selectionA, QString selectionB)
{
    beginResetModel();
    _data = std::move(data);
    _selectionA = std::move(selectionA);
    _selectionB = std::move(selectionB);
    endResetModel();
}

// NOLINTNEXTLINE readability-convert-member-functions-to-static
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
