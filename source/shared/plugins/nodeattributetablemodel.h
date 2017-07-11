#ifndef NODEATTRIBUTETABLEMODEL_H
#define NODEATTRIBUTETABLEMODEL_H

#include "shared/graph/elementid.h"
#include "shared/ui/iselectionmanager.h"
#include "shared/graph/igraphmodel.h"

#include <QAbstractTableModel>
#include <QStringList>
#include <QHash>

#include <set>

class UserNodeData;
class Graph;

class NodeAttributeTableModel : public QAbstractTableModel
{
    Q_OBJECT

    Q_PROPERTY(QStringList columnNames READ columnNames NOTIFY columnNamesChanged)
    Q_PROPERTY(bool showCalculatedAttributes MEMBER _showCalculatedAttributes WRITE showCalculatedAttributes NOTIFY columnNamesChanged)

private:
    const ISelectionManager* _selectionManager = nullptr;
    const UserNodeData* _userNodeData = nullptr;
    const IGraphModel* _graphModel = nullptr;

    enum Roles
    {
        NodeIdRole = Qt::UserRole + 1,
        NodeSelectedRole,
        FirstAttributeRole
    };

    QHash<int, QByteArray> _roleNames;
    QVector<QVariant> _updatedData; // Update occurs here, before being moved to _cachedData
    QVector<QVariant> _cachedData;

    bool _showCalculatedAttributes = true;
    int _columnCount = 0;

    QStringList columnNames() const;
    void update();

private slots:
    void onUpdateComplete();
    void onGraphChanged(const Graph*, bool);

public:
    explicit NodeAttributeTableModel();

    void initialise(ISelectionManager* selectionManager, IGraphModel* graphModel, UserNodeData* userNodeData);

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

    QHash<int, QByteArray> roleNames() const { return _roleNames; }

    void onSelectionChanged();
    void updateRoleNames();
    void showCalculatedAttributes(bool shouldShow);

public slots:
    void onAttributeAdded(const QString& name);
    void onAttributeRemoved(const QString& name);

signals:
    void columnNamesChanged();
};

#endif // NODEATTRIBUTETABLEMODEL_H
