#include "availableattributesmodel.h"

#include "application.h"
#include "graph/graphmodel.h"

AvailableAttributesModel::AvailableAttributesModel(const GraphModel& graphModel,
                                                   QObject* parent,
                                                   ElementType elementTypes,
                                                   ValueType valueTypes) :
    QAbstractItemModel(parent),
    _graphModel(&graphModel)
{
    _attributeList = _graphModel->availableAttributes(elementTypes, valueTypes);
}

QVariant AvailableAttributesModel::data(const QModelIndex& index, int /*role*/) const
{
    return _attributeList.at(index.row());
}

Qt::ItemFlags AvailableAttributesModel::flags(const QModelIndex& index) const
{
    if(!index.isValid())
        return 0;

    return QAbstractItemModel::flags(index);
}

QVariant AvailableAttributesModel::headerData(int /*section*/, Qt::Orientation /*orientation*/, int /*role*/) const
{
    return {};
}

QModelIndex AvailableAttributesModel::index(int row, int column, const QModelIndex& /*parent*/) const
{
    return createIndex(row, column);
}

QModelIndex AvailableAttributesModel::parent(const QModelIndex& /*index*/) const
{
    return {};
}

int AvailableAttributesModel::rowCount(const QModelIndex& /*parent*/) const
{
    return _attributeList.count();
}

int AvailableAttributesModel::columnCount(const QModelIndex& /*parent*/) const
{
    return 1;
}

QVariant AvailableAttributesModel::get(int row) const
{
    return data(index(row, 0), Qt::DisplayRole);
}

void registerAvailableAttributesModelType()
{
    qmlRegisterInterface<AvailableAttributesModel>("AvailableAttributesModel");
}

Q_COREAPP_STARTUP_FUNCTION(registerAvailableAttributesModelType)
