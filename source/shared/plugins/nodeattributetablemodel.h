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

#ifndef NODEATTRIBUTETABLEMODEL_H
#define NODEATTRIBUTETABLEMODEL_H

#include "shared/graph/elementid.h"
#include "shared/ui/idocument.h"
#include "shared/loading/iuserelementdata.h"

#include <QAbstractTableModel>
#include <QStringList>
#include <QHash>
#include <QObject>

#include <vector>
#include <mutex>
#include <deque>

class Graph;
class IGraph;

class NodeAttributeTableModel : public QAbstractTableModel
{
    Q_OBJECT

    Q_PROPERTY(QStringList columnNames MEMBER _columnNames NOTIFY columnNamesChanged)

private:
    IDocument* _document = nullptr;
    const IGraph* _graph = nullptr;
    const IUserNodeData* _userNodeData = nullptr;

    QHash<int, QByteArray> _roleNames;
    std::recursive_mutex _updateMutex;

    using Column = std::vector<QVariant>;
    using Table = std::vector<Column>;

    Column _nodeSelectedColumn;
    Column _nodeIdColumn;

    Table _pendingData; // Update actually occurs here, before being copied to _data on the UI thread
    Table _data;

    QStringList _columnNames;

protected:
    virtual QStringList columnNames() const;
    virtual QVariant dataValue(size_t row, const QString& columnName) const;

    int indexForColumnName(const QString& columnName);

private:
    void updateColumn(int role, Column& column, const QString& columnName = {});
    void update();

private slots:
    void onUpdateComplete();
    void onGraphChanged(const Graph*, bool);

public:
    enum Roles
    {
        NodeIdRole = Qt::UserRole + 1,
        NodeSelectedRole,
        FirstAttributeRole
    };

    void initialise(IDocument* document, IUserNodeData* userNodeData);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    QHash<int, QByteArray> roleNames() const override { return _roleNames; }

    void onSelectionChanged();

    Q_INVOKABLE virtual bool columnIsCalculated(const QString& columnName) const;
    Q_INVOKABLE virtual bool columnIsHiddenByDefault(const QString& columnName) const;
    Q_INVOKABLE void moveFocusToNodeForRowIndex(size_t row);
    Q_INVOKABLE virtual bool columnIsFloatingPoint(const QString& columnName) const;
    Q_INVOKABLE virtual bool columnIsNumerical(const QString& columnName) const;
    Q_INVOKABLE virtual bool rowVisible(size_t row) const;
    Q_INVOKABLE virtual QString columnNameFor(size_t column) const;
    Q_INVOKABLE std::vector<size_t> sourcesOf(const std::vector<size_t>& rows) const;
    Q_INVOKABLE std::vector<size_t> targetsOf(const std::vector<size_t>& rows) const;
    Q_INVOKABLE std::vector<size_t> neighboursOf(const std::vector<size_t>& rows) const;

    void updateColumnNames();

public slots:
    void onAttributesChanged(QStringList added, QStringList removed, QStringList changed, bool graphChanged);

signals:
    void columnNamesChanged();
    void selectionChanged();
};

#endif // NODEATTRIBUTETABLEMODEL_H
