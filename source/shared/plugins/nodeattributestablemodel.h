#ifndef NODEATTRIBUTESTABLEMODEL_H
#define NODEATTRIBUTESTABLEMODEL_H

#include "shared/graph/elementid.h"

#include <QAbstractTableModel>
#include <QStringList>
#include <QHash>

#include <vector>

class NodeAttributes;

class NodeAttributesTableModel : public QAbstractTableModel
{
    Q_OBJECT

    Q_PROPERTY(QStringList columnNames READ columnNames NOTIFY columnNamesChanged)

private:
    const NodeAttributes* _nodeAttributes;
    std::vector<NodeId> _selectedNodeIds;

    int _nextRole = Qt::UserRole + 1;
    QHash<int, QByteArray> _roleNames;

    QStringList columnNames() const;

private slots:
    void onAttributeAdded(const QString& name);

public:
    explicit NodeAttributesTableModel(NodeAttributes* attributes);

    void setSelectedNodes(const NodeIdSet& selectedNodeIds);

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

    QHash<int, QByteArray> roleNames() const { return _roleNames; }

signals:
    void columnNamesChanged();
};

#endif // NODEATTRIBUTESTABLEMODEL_H
