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

#ifndef EDITATTRIBUTETABLEMODEL_H
#define EDITATTRIBUTETABLEMODEL_H

#include "app/attributes/attributeedits.h"

#include "shared/graph/elementid.h"
#include "shared/attributes/iattribute.h"

#include <QObject>
#include <QQmlEngine>
#include <QString>
#include <QAbstractTableModel>
#include <QVariant>

#include <vector>
#include <map>

class Document;
Q_DECLARE_OPAQUE_POINTER(Document*)

class SelectionManager;

class EditAttributeTableModel : public QAbstractTableModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(Document* document MEMBER _document WRITE setDocument NOTIFY documentChanged)
    Q_PROPERTY(QString attributeName MEMBER _attributeName WRITE setAttributeName NOTIFY attributeNameChanged)
    Q_PROPERTY(bool hasEdits READ hasEdits NOTIFY hasEditsChanged)
    Q_PROPERTY(AttributeEdits edits READ edits NOTIFY editsChanged)
    Q_PROPERTY(bool combineSharedValues MEMBER _combineSharedValues NOTIFY combineSharedValuesChanged)
    Q_PROPERTY(int sortColumn MEMBER _sortColumn NOTIFY sortColumnChanged)
    Q_PROPERTY(bool ascendingSortOrder MEMBER _ascendingSortOrder NOTIFY ascendingSortOrderChanged)

public:
    EditAttributeTableModel();

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;

    QHash<int, QByteArray> roleNames() const override { return _roleNames; }

    Q_INVOKABLE void editValue(int row, const QString& value);
    Q_INVOKABLE void resetRowValue(int row);
    Q_INVOKABLE void resetAllEdits();

    Q_INVOKABLE bool rowIsEdited(int row) const;

private:
    Document* _document = nullptr;
    QString _attributeName;
    const IAttribute* _attribute = nullptr;
    std::vector<IAttribute::SharedValue> _attributeSharedValues;
    bool _combineSharedValues = false;

    std::vector<int> _sortMap;
    int _sortColumn = 0;
    bool _ascendingSortOrder = true;

    std::vector<NodeId> _selectedNodes;

    std::map<int, std::vector<NodeId>> _nodeIdsMap;
    std::map<int, std::vector<EdgeId>> _edgeIdsMap;

    AttributeEdits _edits;

    enum Roles
    {
        LabelRole = Qt::UserRole + 1,
        AttributeRole,
        EditedRole
    };

    QHash<int, QByteArray> _roleNames =
    {
        {Qt::DisplayRole,   "display"},

        // These are only exposed to facilitate sorting further downstream
        {LabelRole,         "label"},
        {AttributeRole,     "attribute"},
        {EditedRole,        "edited"},
    };

    NodeId rowToNodeId(int row) const;
    QVariant valueAt(int column, int row) const;

    void setDocument(Document* document);
    void setAttributeName(const QString& attributeName);

    bool hasEdits() const { return !_edits._nodeValues.empty() || !_edits._edgeValues.empty(); }

    const AttributeEdits& edits() const { return _edits; }

private slots:
    void updateSortMap();
    void onSelectionChanged();

signals:
    void documentChanged();
    void attributeNameChanged();
    void hasEditsChanged();
    void editsChanged();
    void combineSharedValuesChanged();
    void sortColumnChanged();
    void ascendingSortOrderChanged();
};

#endif // EDITATTRIBUTETABLEMODEL_H
