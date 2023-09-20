/* Copyright © 2013-2023 Graphia Technologies Ltd.
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

#ifndef TABLEPROXYMODEL_H
#define TABLEPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QTimer>
#include <QItemSelectionRange>
#include <QStandardItemModel>
#include <QCollator>
#include <QStringList>

#include <vector>
#include <map>
#include <unordered_set>
#include <deque>
#include <utility>

// As QSortFilterProxyModel cannot set column orders, we do it ourselves by translating columns
// in the data() function. This has a number of consequences regarding proxy/source mappings.

// Interface mapFromSource() and mapToSource() will NOT account for column ordering.
// As such order mapping is maintained via orderedProxyToSourceColumn
// This can be accessed via mapToOrderedSourceColumn()

// The sort() function requires an UNORDERED column as translation is taken care of in the data()
// function. Passing sort() an ordered column index will result in an incorrect translation.
// Unordered mapped columns can be accessed via the _unorderedSourceToProxyColumn member
// Sort column is stored as a SOURCE index to respect ordering and hidden columns.

class TableProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(QAbstractItemModel* headerModel READ headerModel CONSTANT)
    Q_PROPERTY(QStringList columnNames MEMBER _columnNames NOTIFY columnNamesChanged)
    Q_PROPERTY(QStringList hiddenColumns MEMBER _hiddenColumns WRITE setHiddenColumns)
    Q_PROPERTY(QStringList columnOrder MEMBER _sourceColumnOrder WRITE setColumnOrder NOTIFY columnOrderChanged)
    Q_PROPERTY(QString sortColumn READ sortColumn_ WRITE setSortColumn NOTIFY sortColumnChanged)
    Q_PROPERTY(Qt::SortOrder sortOrder READ sortOrder_ WRITE setSortOrder NOTIFY sortOrderChanged)
    Q_PROPERTY(bool isRowOrderSet READ isRowOrderSet NOTIFY isRowOrderSetChanged)

private:
    QStandardItemModel _headerModel;
    QStringList _columnNames;
    bool _showCalculatedColumns = false;
    QItemSelection _subSelection;
    std::unordered_set<int> _subSelectionRows;
    QStringList _hiddenColumns;
    QStringList _sourceColumnOrder;
    std::vector<int> _orderedProxyToSourceColumn;

    QCollator _collator;
    std::deque<std::pair<QString, Qt::SortOrder>> _sortColumnAndOrders;
    std::map<size_t, size_t> _rowOrderMap;

    enum Roles
    {
        SubSelectedRole = Qt::UserRole + 999
    };

    QAbstractItemModel* headerModel() { return &_headerModel; }
    void calculateOrderedProxySourceMapping();
    void updateSourceModelFilter();

    void resort();

    void onColumnNamesChanged();

    bool isRowOrderSet() const;

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
    bool filterAcceptsColumn(int sourceColumn, const QModelIndex& sourceParent) const override;
    bool lessThan(const QModelIndex& a, const QModelIndex& b) const override;

public:
    QVariant data(const QModelIndex& index, int role) const override;

    QHash<int, QByteArray> roleNames() const override
    {
        auto roleNames = sourceModel()->roleNames();
        roleNames.insert(Roles::SubSelectedRole, "subSelected");
        return roleNames;
    }

    explicit TableProxyModel(QObject* parent = nullptr);
    Q_INVOKABLE void setSubSelection(const QItemSelection& subSelection, const QItemSelection& subDeselection);
    Q_INVOKABLE int mapToSourceRow(int proxyRow) const;
    Q_INVOKABLE int mapFromSourceRow(int sourceRow) const;
    Q_INVOKABLE int mapOrderedToSourceColumn(int proxyColumn) const;
    Q_INVOKABLE QItemSelectionRange buildRowSelectionRange(int topRow, int bottomRow);
    Q_INVOKABLE QItemSelection buildRowSelection(const std::vector<int>& rows);

    Q_INVOKABLE void setRowOrder(const std::vector<int>& rows);

    using QSortFilterProxyModel::mapToSource;

    void setHiddenColumns(const QStringList& hiddenColumns);
    void setColumnOrder(const QStringList& columnOrder);

    // The underscores are to avoid clashes with QSortFilterProxyModel::sort[Column|Order]
    QString sortColumn_() const;
    void setSortColumn(const QString& newSortColumn);

    Qt::SortOrder sortOrder_() const;
    void setSortOrder(Qt::SortOrder newSortOrder);

signals:
    void columnNamesChanged();

    void sortColumnChanged(const QString& sortColumn);
    void sortOrderChanged(Qt::SortOrder sortOrder);
    void isRowOrderSetChanged();

    void filterRoleNameChanged();
    void filterPatternSyntaxChanged();
    void filterPatternChanged();
    void filterValueChanged();

    void showCalculatedColumnsChanged();
    void columnOrderChanged();

public slots:
    void invalidateFilter();
};

#endif // TABLEPROXYMODEL_H
