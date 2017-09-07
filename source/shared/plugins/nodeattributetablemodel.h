#ifndef NODEATTRIBUTETABLEMODEL_H
#define NODEATTRIBUTETABLEMODEL_H

#include "shared/graph/elementid.h"
#include "shared/ui/idocument.h"

#include <QAbstractTableModel>
#include <QStringList>
#include <QHash>

#include <set>
#include <mutex>
#include <deque>

class UserNodeData;
class Graph;

class NodeAttributeTableModel : public QAbstractTableModel
{
    Q_OBJECT

    Q_PROPERTY(QStringList columnNames READ columnNames NOTIFY columnNamesChanged)

private:
    IDocument* _document = nullptr;
    const UserNodeData* _userNodeData = nullptr;

    enum Roles
    {
        NodeIdRole = Qt::UserRole + 1,
        NodeSelectedRole,
        FirstAttributeRole
    };

    QHash<int, QByteArray> _roleNames;
    std::mutex _updateMutex;

    using Row = std::vector<QVariant>;
    using Table = std::vector<Row>;

    std::deque<Table> _updatedData; // Update occurs here, before being moved to _cachedData
    Table _cachedData;

    int _columnCount = 0;

    QStringList columnNames() const;
    void update();

private slots:
    void onUpdateComplete();
    void onGraphChanged(const Graph*, bool);

public:
    explicit NodeAttributeTableModel();

    void initialise(IDocument* document, UserNodeData* userNodeData);

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

    QHash<int, QByteArray> roleNames() const { return _roleNames; }

    void onSelectionChanged();
    void updateRoleNames();

    Q_INVOKABLE bool columnIsCalculated(const QString& columnName);

    Q_INVOKABLE void focusNodeForRowIndex(size_t row);

public slots:
    void onAttributeAdded(const QString& name);
    void onAttributeRemoved(const QString& name);

signals:
    void columnNamesChanged();
};

#endif // NODEATTRIBUTETABLEMODEL_H
