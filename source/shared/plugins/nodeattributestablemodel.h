#ifndef NODEATTRIBUTESTABLEMODEL_H
#define NODEATTRIBUTESTABLEMODEL_H

#include "shared/graph/elementid.h"
#include "shared/ui/iselectionmanager.h"

#include <QAbstractTableModel>
#include <QStringList>
#include <QHash>

#include <set>

class NodeAttributes;

class NodeAttributesTableModel : public QAbstractTableModel
{
    Q_OBJECT

    Q_PROPERTY(QStringList columnNames READ columnNames NOTIFY columnNamesChanged)

private:
    const ISelectionManager* _selectionManager = nullptr;
    const NodeAttributes* _nodeAttributes = nullptr;

    const int _nodeIdRole = Qt::UserRole + 1;
    const int _nodeSelectedRole = Qt::UserRole + 2;
    int _nextRole = Qt::UserRole + 3;
    QHash<int, QByteArray> _roleNames;

    QStringList columnNames() const;

private slots:
    void onAttributeAdded(const QString& name);

public:
    explicit NodeAttributesTableModel(NodeAttributes* attributes);

    void initialise(ISelectionManager* selectionManager);

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

    QHash<int, QByteArray> roleNames() const { return _roleNames; }

    void onSelectionChanged();

signals:
    void columnNamesChanged();
};

#endif // NODEATTRIBUTESTABLEMODEL_H
