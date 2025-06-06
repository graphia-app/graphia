/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

#include "shared/utils/string.h"

#include <QQmlEngine>
#include <QDebug>

using namespace Qt::Literals::StringLiterals;

EnrichmentTableModel::EnrichmentTableModel(QObject *parent)
{
    setParent(parent);

    constexpr int RoleBase = Qt::UserRole;

    _roleNames[RoleBase + Results::SelectionA] = "SelectionA";
    _roleNames[RoleBase + Results::SelectionB] = "SelectionB";
    _roleNames[RoleBase + Results::Observed] = "Observed";
    _roleNames[RoleBase + Results::ExpectedTrial] = "ExpectedTrial";
    _roleNames[RoleBase + Results::OverRep] = "OverRep";
    _roleNames[RoleBase + Results::Fishers] = "Fishers";
    _roleNames[RoleBase + Results::BonferroniAdjusted] = "BonferroniAdjusted";

    connect(this, &EnrichmentTableModel::enrichedOnlyChanged, this, &EnrichmentTableModel::updateSortFilterMap);
    connect(this, &EnrichmentTableModel::sortColumnChanged, this, &EnrichmentTableModel::updateSortFilterMap);
    connect(this, &EnrichmentTableModel::ascendingSortOrderChanged, this, &EnrichmentTableModel::updateSortFilterMap);
}

int EnrichmentTableModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return static_cast<int>(_sortFilterMap.size() + 1 /* header */);
}

int EnrichmentTableModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return Results::NumResultColumns;
}

static bool enriched(const EnrichmentTableModel::Row& dataRow)
{
    const auto overRep = dataRow.at(static_cast<size_t>(EnrichmentTableModel::Results::OverRep)).toDouble();
    const auto bonferroni = dataRow.at(static_cast<size_t>(EnrichmentTableModel::Results::BonferroniAdjusted)).toDouble();

    return overRep > 1.0 && bonferroni <= 0.05;
}

QVariant EnrichmentTableModel::data(const QModelIndex& index, int role) const
{
    auto row = static_cast<size_t>(index.row());
    Q_ASSERT(row < static_cast<size_t>(rowCount()));

    auto sortedFilteredRow = row > 0 ? _sortFilterMap.at(row - 1) : 0;

    if(role == EnrichedRole)
    {
        // Ignore header for this role
        if(row == 0)
            return true;

        return enriched(_data.at(sortedFilteredRow));
    }

    auto column = static_cast<size_t>(index.column());
    Q_ASSERT(column < static_cast<size_t>(columnCount()));

    if(row == 0)
    {
        switch(static_cast<Results>(column))
        {
        case Results::SelectionA:           return _selectionA.isEmpty() ? u"Selection A"_s : _selectionA;
        case Results::SelectionB:           return _selectionB.isEmpty() ? u"Selection B"_s : _selectionB;
        case Results::Observed:             return u"Observed"_s;
        case Results::ExpectedTrial:        return u"Expected"_s;
        case Results::OverRep:              return u"Representation"_s;
        case Results::Fishers:              return u"Fishers"_s;
        case Results::BonferroniAdjusted:   return u"Bonferroni Adjusted"_s;
        default:
            return {};
        }
    }

    return _data.at(sortedFilteredRow).at(column);
}

QVariant EnrichmentTableModel::data(int row, Results result) const
{
    auto column = static_cast<int>(result);
    return data(index(row, column), Qt::DisplayRole);
}

int EnrichmentTableModel::rowFromAttributeSets(const QString& attributeA, const QString& attributeB) const
{
    for(int rowIndex = 1; rowIndex < rowCount(); rowIndex++)
    {
        if(data(rowIndex, Results::SelectionA) == attributeA &&
            data(rowIndex, Results::SelectionB) == attributeB)
        {
            return rowIndex;
        }
    }

    return -1;
}

QHash<int, QByteArray> EnrichmentTableModel::roleNames() const
{
    return
    {
        {Qt::DisplayRole, "display"},
        {EnrichedRole, "enriched"}
    };
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
    updateSortFilterMap();
    endResetModel();
}

void EnrichmentTableModel::updateSortFilterMap()
{
    beginResetModel();

    _sortFilterMap.clear();

    if(_enrichedOnly)
    {
        for(size_t i = 0; i < _data.size(); i++)
        {
            if(enriched(_data.at(i)))
                _sortFilterMap.push_back(i);
        }
    }
    else
    {
        _sortFilterMap.resize(_data.size());
        std::iota(_sortFilterMap.begin(), _sortFilterMap.end(), 0);
    }

    if(_sortColumn >= 0)
    {
        std::sort(_sortFilterMap.begin(), _sortFilterMap.end(), [this](size_t rowA, size_t rowB)
        {
            if(!_ascendingSortOrder)
                std::swap(rowA, rowB);

            auto valueA = _data.at(rowA).at(static_cast<size_t>(_sortColumn));
            auto valueB = _data.at(rowB).at(static_cast<size_t>(_sortColumn));

            if(valueA.userType() == QMetaType::QString && valueB.userType() == QMetaType::QString)
                return u::numericCompare(valueA.toString(), valueB.toString()) < 0;

            return QVariant::compare(valueA, valueB) == QPartialOrdering::Less;
        });
    }

    endResetModel();
}
