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

#include "qmlsortfilterproxymodel.h"

QmlSortFilterProxyModel::QmlSortFilterProxyModel(QObject* parent) : QSortFilterProxyModel(parent)
{
    connect(this, &QmlSortFilterProxyModel::sourceModelChanged, this, &QmlSortFilterProxyModel::updateFilterRole);
    connect(this, &QmlSortFilterProxyModel::sourceModelChanged, this, &QmlSortFilterProxyModel::updateSortRole);

    connect(this, &QmlSortFilterProxyModel::sourceModelChanged, this, &QmlSortFilterProxyModel::countChanged);
    connect(this, &QmlSortFilterProxyModel::rowsInserted, this, &QmlSortFilterProxyModel::countChanged);
    connect(this, &QmlSortFilterProxyModel::rowsRemoved, this, &QmlSortFilterProxyModel::countChanged);
    connect(this, &QmlSortFilterProxyModel::modelReset, this, &QmlSortFilterProxyModel::countChanged);
    connect(this, &QmlSortFilterProxyModel::layoutChanged, this, &QmlSortFilterProxyModel::countChanged);

    connect(this, &QmlSortFilterProxyModel::sourceModelChanged, this, &QmlSortFilterProxyModel::invalidate);
    connect(this, &QmlSortFilterProxyModel::ascendingSortOrderChanged, this, &QmlSortFilterProxyModel::invalidate);

    sort(0);
}

QVariant QmlSortFilterProxyModel::get(int row, const QString& roleName) const
{
    return data(index(row, 0), role(roleName));
}

int QmlSortFilterProxyModel::role(const QString& name) const
{
    if(sourceModel() == nullptr)
        return -1;

    if(name.isEmpty())
        return Qt::DisplayRole;

    auto value = name.toUtf8();
    return sourceModel()->roleNames().key(value);
}

bool QmlSortFilterProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
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

bool QmlSortFilterProxyModel::filterAcceptsRow(int row, const QModelIndex& parent) const
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

void QmlSortFilterProxyModel::setSortRoleName(const QString& sortRoleName)
{
    _sortRoleName = sortRoleName;
    updateSortRole();
    emit sortRoleNameChanged();
}

QJSValue QmlSortFilterProxyModel::sortExpression() const
{
    return _sortExpression;
}

void QmlSortFilterProxyModel::setSortExpression(const QJSValue& sortExpression)
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

void QmlSortFilterProxyModel::setFilterRoleName(const QString& filterRoleName)
{
    _filterRoleName = filterRoleName;
    updateFilterRole();
    emit filterRoleNameChanged();
}

QJSValue QmlSortFilterProxyModel::filterExpression() const
{
    return _filterExpression;
}

void QmlSortFilterProxyModel::setFilterExpression(const QJSValue& filterExpression)
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

int QmlSortFilterProxyModel::count() const
{
    if(sourceModel() == nullptr)
        return 0;

    return sourceModel()->rowCount();
}

void QmlSortFilterProxyModel::updateSortRole()
{
    setSortRole(role(_sortRoleName));
}

void QmlSortFilterProxyModel::updateFilterRole()
{
    setFilterRole(role(_filterRoleName));
}
