#ifndef NODEATTRIBUTETABLEMODEL_H
#define NODEATTRIBUTETABLEMODEL_H

#include "shared/graph/elementid.h"
#include "shared/ui/idocument.h"
#include "shared/plugins/userelementdata.h"

#include <QAbstractTableModel>
#include <QStringList>
#include <QHash>
#include <QObject>

#include <set>
#include <mutex>
#include <deque>

class Graph;

class NodeAttributeTableModel : public QAbstractTableModel
{
    Q_OBJECT

    Q_PROPERTY(QStringList columnNames MEMBER _columnNames NOTIFY columnNamesChanged)

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

    QStringList _columnNames;
    int _columnCount = 0;

protected:
    virtual QStringList columnNames() const;
    virtual QVariant dataValue(int row, int role) const;

private:
    void update();

private slots:
    void onUpdateComplete();
    void onGraphChanged(const Graph*, bool);

public:
    void initialise(IDocument* document, UserNodeData* userNodeData);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    QHash<int, QByteArray> roleNames() const override { return _roleNames; }

    void onSelectionChanged();
    virtual void updateRoleNames();

    Q_INVOKABLE virtual bool columnIsCalculated(const QString& columnName) const;
    Q_INVOKABLE virtual bool columnIsHiddenByDefault(const QString& columnName) const;
    Q_INVOKABLE void moveFocusToNodeForRowIndex(size_t row);
    Q_INVOKABLE virtual bool columnIsFloatingPoint(const QString& columnName) const;

public slots:
    void onAttributeAdded(const QString& name);
    void onAttributeRemoved(const QString& name);

signals:
    void columnNamesChanged();
};

#endif // NODEATTRIBUTETABLEMODEL_H
