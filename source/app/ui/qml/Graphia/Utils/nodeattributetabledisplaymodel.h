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

#ifndef NODEATTRIBUTETABLEDISPLAYMODEL_H
#define NODEATTRIBUTETABLEDISPLAYMODEL_H

#include <QQmlEngine>
#include <QAbstractTableModel>
#include <QList>
#include <QStandardItemModel>
#include <QStringList>
#include <QByteArray>
#include <QHash>
#include <QObject>
#include <QString>
#include <QVariant>

#include <vector>
#include <unordered_map>
#include <deque>
#include <utility>

class NodeAttributeTableModel;

// What a NodeAttributeTableView shows of a NodeAttributeTableModel: a sorted row for each
// selected node, the visible columns in order, and which rows the user has selected. Outside
// of here, rows are identified by their row in the source model, i.e. the NodeAttributeTableModel

class NodeAttributeTableDisplayModel : public QAbstractTableModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QAbstractItemModel* sourceModel READ sourceModel WRITE setSourceModel NOTIFY sourceModelChanged)
    Q_PROPERTY(QAbstractItemModel* headerModel READ headerModel CONSTANT)
    Q_PROPERTY(QStringList hiddenColumns MEMBER _hiddenColumns WRITE setHiddenColumns)
    Q_PROPERTY(QStringList columnOrder MEMBER _columnOrder WRITE setColumnOrder NOTIFY columnOrderChanged)
    Q_PROPERTY(QString sortColumn READ sortColumn WRITE setSortColumn NOTIFY sortColumnChanged)
    Q_PROPERTY(Qt::SortOrder sortOrder READ sortOrder WRITE setSortOrder NOTIFY sortOrderChanged)
    Q_PROPERTY(bool isRowOrderSet READ isRowOrderSet NOTIFY isRowOrderSetChanged)
    Q_PROPERTY(QVariantList sourceRows READ sourceRowsAsVariantList NOTIFY sourceRowsChanged)
    Q_PROPERTY(int numRows READ numRows NOTIFY sourceRowsChanged)
    Q_PROPERTY(QList<int> selectedRows READ selectedRows NOTIFY selectedRowsChanged)

private:
    NodeAttributeTableModel* _sourceModel = nullptr;

    QStandardItemModel _headerModel;
    QStringList _columnNames;
    QStringList _hiddenColumns;
    QStringList _columnOrder;

    // Display column to source column
    std::vector<int> _sourceColumns;

    // Display row to source row, and the inverse
    std::vector<int> _sourceRows;
    std::vector<int> _rowForSourceRow;

    std::deque<std::pair<QString, Qt::SortOrder>> _sortColumnAndOrders;
    std::unordered_map<int, size_t> _rowOrderMap;

    std::vector<bool> _rowSelected;
    QList<int> _selectedRows;

    enum Roles // NOLINT performance-enum-size
    {
        SubSelectedRole = Qt::UserRole + 999
    };

    QAbstractItemModel* sourceModel() const;
    void setSourceModel(QAbstractItemModel* sourceModel);

    QAbstractItemModel* headerModel() { return &_headerModel; }

    void updateSourceColumns();
    void sortSourceRows();
    bool updateSourceRows();

    void reset();
    void resort();

    void onSourceSelectionChanged();
    void onSourceDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QList<int>& roles);
    void onSourceModelReset();

    bool isRowOrderSet() const;

    QVariantList sourceRowsAsVariantList() const;
    int numRows() const { return rowCount(); }

    QList<int> selectedRows() const { return _selectedRows; }

    template<typename Fn> void changeSelection(const Fn& fn);
    void publishSelection();

public:
    explicit NodeAttributeTableDisplayModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE int sourceRowFor(int row) const;
    Q_INVOKABLE int sourceColumnFor(int column) const;

    Q_INVOKABLE bool isRowSelected(int row) const;
    Q_INVOKABLE void selectRows(int firstRow, int lastRow, bool clearFirst = false);
    Q_INVOKABLE void deselectRows(int firstRow, int lastRow);
    Q_INVOKABLE void selectSourceRows(const std::vector<int>& sourceRows, bool clearFirst = false);

    Q_INVOKABLE void setRowOrder(const std::vector<int>& sourceRows);

    void setHiddenColumns(const QStringList& hiddenColumns);
    void setColumnOrder(const QStringList& columnOrder);

    QString sortColumn() const;
    void setSortColumn(const QString& newSortColumn);

    Qt::SortOrder sortOrder() const;
    void setSortOrder(Qt::SortOrder newSortOrder);

signals:
    void sourceModelChanged();

    void sortColumnChanged(const QString& sortColumn);
    void sortOrderChanged(Qt::SortOrder sortOrder);
    void isRowOrderSetChanged();
    void sourceRowsChanged();
    void selectedRowsChanged();
    void rowsReplaced();

    void columnOrderChanged();
};

#endif // NODEATTRIBUTETABLEDISPLAYMODEL_H
