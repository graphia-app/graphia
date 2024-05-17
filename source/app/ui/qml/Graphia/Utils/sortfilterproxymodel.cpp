/* Copyright © 2013-2024 Graphia Technologies Ltd.
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

#include "sortfilterproxymodel.h"

SortFilterProxyModel::SortFilterProxyModel(QObject* parent) : QSortFilterProxyModel(parent)
{
    connect(this, &SortFilterProxyModel::sourceModelChanged, this, &SortFilterProxyModel::updateFilterRole);
    connect(this, &SortFilterProxyModel::sourceModelChanged, this, &SortFilterProxyModel::updateSortRole);

    connect(this, &SortFilterProxyModel::sourceModelChanged, this, &SortFilterProxyModel::countChanged);
    connect(this, &SortFilterProxyModel::rowsInserted, this, &SortFilterProxyModel::countChanged);
    connect(this, &SortFilterProxyModel::rowsRemoved, this, &SortFilterProxyModel::countChanged);
    connect(this, &SortFilterProxyModel::modelReset, this, &SortFilterProxyModel::countChanged);
    connect(this, &SortFilterProxyModel::layoutChanged, this, &SortFilterProxyModel::countChanged);

    connect(this, &SortFilterProxyModel::sourceModelChanged, this, &SortFilterProxyModel::invalidate);
    connect(this, &SortFilterProxyModel::ascendingSortOrderChanged, this, &SortFilterProxyModel::invalidate);

    sort(0);
}

QVariant SortFilterProxyModel::get(int row, const QString& roleName) const
{
    return data(index(row, 0), role(roleName));
}

int SortFilterProxyModel::role(const QString& name) const
{
    if(sourceModel() == nullptr)
        return -1;

    if(name.isEmpty())
        return Qt::DisplayRole;

    auto value = name.toUtf8();
    return sourceModel()->roleNames().key(value);
}

bool SortFilterProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    if(!left.isValid())
        return false;

    if(!right.isValid())
        return true;

    bool result;

    auto* engine = qjsEngine(this);
    if(engine != nullptr && _sortExpression.isCallable())
    {
        auto leftValue = engine->toScriptValue(left);
        auto rightValue = engine->toScriptValue(right);
        result = _sortExpression.call({leftValue, rightValue}).toBool();
    }
    else
        result = QSortFilterProxyModel::lessThan(left, right);

    return _ascendingSortOrder ? result : !result;
}

bool SortFilterProxyModel::filterAcceptsRow(int row, const QModelIndex& parent) const
{
    auto* engine = qjsEngine(this);
    if(engine != nullptr && _filterExpression.isCallable())
    {
        auto parentValue = engine->toScriptValue(parent);
        auto value = _filterExpression.call({row, parentValue});
        return value.toBool();
    }

    return QSortFilterProxyModel::filterAcceptsRow(row, parent);
}

void SortFilterProxyModel::setSortRoleName(const QString& sortRoleName)
{
    _sortRoleName = sortRoleName;
    updateSortRole();
    emit sortRoleNameChanged();
}

QJSValue SortFilterProxyModel::sortExpression() const
{
    return _sortExpression;
}

void SortFilterProxyModel::setSortExpression(const QJSValue& sortExpression)
{
    if(!sortExpression.isNull() && !sortExpression.isUndefined() && !sortExpression.isCallable())
    {
        qWarning() << this << "setSortExpression called on non-expression";
        return;
    }

    if(sortExpression.strictlyEquals(_sortExpression))
        return;

    _sortExpression = sortExpression;
    emit sortExpressionChanged();
    invalidate();
}

void SortFilterProxyModel::setFilterRoleName(const QString& filterRoleName)
{
    _filterRoleName = filterRoleName;
    updateFilterRole();
    emit filterRoleNameChanged();
}

QJSValue SortFilterProxyModel::filterExpression() const
{
    return _filterExpression;
}

void SortFilterProxyModel::setFilterExpression(const QJSValue& filterExpression)
{
    if(!filterExpression.isNull() && !filterExpression.isUndefined() && !filterExpression.isCallable())
    {
        qWarning() << this << "filterExpression called on non-expression";
        return;
    }

    if(filterExpression.strictlyEquals(_filterExpression))
        return;

    _filterExpression = filterExpression;
    emit filterExpressionChanged();
    invalidate();
}

int SortFilterProxyModel::count() const
{
    return rowCount();
}

void SortFilterProxyModel::updateSortRole()
{
    setSortRole(role(_sortRoleName));
}

void SortFilterProxyModel::updateFilterRole()
{
    setFilterRole(role(_filterRoleName));
}
