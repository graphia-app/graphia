#ifndef AVAILABLEATTRIBUTESMODEL_H
#define AVAILABLEATTRIBUTESMODEL_H

#include "shared/graph/elementtype.h"
#include "shared/attributes/valuetype.h"

#include <QAbstractItemModel>
#include <QList>
#include <QVariant>

class GraphModel;

class AvailableAttributesModel : public QAbstractItemModel
{
    Q_OBJECT

private:
    class Item
    {
    private:
        QVariant _value;

        QList<Item*> _children;
        Item* _parent = nullptr;

    public:
        explicit Item(QVariant value);
        virtual ~Item();

        void addChild(Item* child);

        Item* child(int row);
        int childCount() const;
        QVariant value() const;
        int row() const;
        Item* parent();
    };

    Item* _root = nullptr;

    // When listing edge attributes, we want their source and target node attributes too
    Item* _sourceNode = nullptr;
    Item* _targetNode = nullptr;

    Item* parentItem(const QModelIndex& index) const;

    enum Roles
    {
        ElementTypeRole = Qt::UserRole + 1,
        ValueTypeRole,
        HasSharedValuesRole,
        SearchableRole,
        UserDefinedRole
    };

    const GraphModel* _graphModel = nullptr;

public:
    AvailableAttributesModel() = default;
    explicit AvailableAttributesModel(const GraphModel& graphModel,
                             QObject* parent = nullptr,
                             ElementType elementTypes = ElementType::All,
                             ValueType valueTypes = ValueType::All);
    ~AvailableAttributesModel() override;

    QVariant data(const QModelIndex& index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex& parentIndex = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parentIndex = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    Q_INVOKABLE QVariant get(const QModelIndex& index) const;

    QHash<int, QByteArray> roleNames() const override;
};

#endif // AVAILABLEATTRIBUTESMODEL_H
