#ifndef AVAILABLEATTRIBUTESMODEL_H
#define AVAILABLEATTRIBUTESMODEL_H

#include "graph/elementtype.h"
#include "attributes/valuetype.h"

#include <QAbstractItemModel>

class GraphModel;

class AvailableAttributesModel : public QAbstractItemModel
{
    Q_OBJECT

private:
    const GraphModel* _graphModel = nullptr;
    QStringList _attributeList;

public:
    AvailableAttributesModel() = default;
    AvailableAttributesModel(const GraphModel& graphModel,
                             QObject* parent = nullptr,
                             ElementType elementTypes = ElementType::All,
                             ValueType valueTypes = ValueType::All);

    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    Q_INVOKABLE QVariant get(int row) const;
};

#endif // AVAILABLEATTRIBUTESMODEL_H
