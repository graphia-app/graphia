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

#ifndef QMLSORTFILTERPROXYMODEL_H
#define QMLSORTFILTERPROXYMODEL_H

#include "shared/utils/static_block.h"

#include <QSortFilterProxyModel>
#include <QQmlEngine>
#include <QJSValue>

class QmlSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(QString sortRoleName MEMBER _sortRoleName WRITE setSortRoleName NOTIFY sortRoleNameChanged)
    Q_PROPERTY(QJSValue sortExpression READ sortExpression WRITE setSortExpression NOTIFY sortExpressionChanged)
    Q_PROPERTY(bool ascendingSortOrder MEMBER _ascendingSortOrder NOTIFY ascendingSortOrderChanged)
    Q_PROPERTY(QString filterRoleName MEMBER _filterRoleName WRITE setFilterRoleName NOTIFY filterRoleNameChanged)
    Q_PROPERTY(QJSValue filterExpression READ filterExpression WRITE setFilterExpression NOTIFY filterExpressionChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    explicit QmlSortFilterProxyModel(QObject* parent = nullptr);

    Q_INVOKABLE QVariant get(int row, const QString& roleName = {}) const;
    Q_INVOKABLE int role(const QString& name) const;

private:
    QString _sortRoleName;
    QJSValue _sortExpression;
    bool _ascendingSortOrder = true;
    QString _filterRoleName;
    QJSValue _filterExpression;

    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;
    bool filterAcceptsRow(int row, const QModelIndex &parent) const override;

    void setSortRoleName(const QString& sortRoleName);
    QJSValue sortExpression() const;
    void setSortExpression(const QJSValue& sortExpression);

    void setFilterRoleName(const QString& filterRoleName);
    QJSValue filterExpression() const;
    void setFilterExpression(const QJSValue& filterExpression);

    int count() const;

private slots:
    void updateSortRole();
    void updateFilterRole();

signals:
    void sortRoleNameChanged();
    void sortExpressionChanged();
    void ascendingSortOrderChanged();
    void filterRoleNameChanged();
    void filterExpressionChanged();
    void countChanged();
};

static_block
{
    qmlRegisterType<QmlSortFilterProxyModel>(
        APP_URI, APP_MAJOR_VERSION, APP_MINOR_VERSION, "SimpleSortFilterProxyModel");
}

#endif // QMLSORTFILTERPROXYMODEL_H
