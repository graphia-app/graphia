#include "availableattributesmodel.h"

#include "application.h"
#include "graph/graphmodel.h"
#include "shared/graph/elementtype.h"

AvailableAttributesModel::Item::Item(const QVariant& value) :
    _value(value)
{}

AvailableAttributesModel::Item::~Item()
{
    qDeleteAll(_children);
}

void AvailableAttributesModel::Item::addChild(AvailableAttributesModel::Item* child)
{
    _children.append(child);
    child->_parent = this;
}

AvailableAttributesModel::Item*AvailableAttributesModel::Item::child(int row)
{
    return _children.value(row);
}

int AvailableAttributesModel::Item::childCount() const
{
    return _children.count();
}

QVariant AvailableAttributesModel::Item::value() const
{
    return _value;
}

int AvailableAttributesModel::Item::row() const
{
    if(_parent != nullptr)
        return _parent->_children.indexOf(const_cast<Item*>(this));

    return 0;
}

AvailableAttributesModel::Item*AvailableAttributesModel::Item::parent()
{
    return _parent;
}

AvailableAttributesModel::AvailableAttributesModel(const GraphModel& graphModel,
                                                   QObject* parent,
                                                   ElementType elementTypes,
                                                   ValueType valueTypes) :
    QAbstractItemModel(parent),
    _graphModel(&graphModel)
{
    _root = new AvailableAttributesModel::Item(tr("Attribute"));

    auto attributeList = graphModel.availableAttributes(elementTypes, valueTypes);

    for(const auto& attribute : attributeList)
        _root->addChild(new AvailableAttributesModel::Item(attribute));

    if(elementTypes == ElementType::Edge)
    {
        attributeList = graphModel.availableAttributes(ElementType::Node, valueTypes);

        if(!attributeList.empty())
        {
            _sourceNode = new AvailableAttributesModel::Item(tr("Source Node"));
            _targetNode = new AvailableAttributesModel::Item(tr("Target Node"));
            _root->addChild(_sourceNode);
            _root->addChild(_targetNode);

            for(const auto& attribute : attributeList)
            {
                _sourceNode->addChild(new AvailableAttributesModel::Item(attribute));
                _targetNode->addChild(new AvailableAttributesModel::Item(attribute));
            }
        }
    }
}

AvailableAttributesModel::~AvailableAttributesModel()
{
    delete _root;
}

QVariant AvailableAttributesModel::data(const QModelIndex& index, int role) const
{
    if(!index.isValid())
        return {};

    auto* item = static_cast<AvailableAttributesModel::Item*>(index.internalPointer());

    auto itemvalue = item->value().toString();

    if(role != Qt::DisplayRole)
    {
        // Special case for source/target node items, which don't have sections
        if(item->childCount() > 0)
            return {};

        auto attribute = _graphModel->attributeByName(itemvalue);

        if(attribute == nullptr)
            return {};

        switch(role)
        {
        case Roles::ElementTypeRole:
        {
            auto asString = elementTypeAsString(attribute->elementType());
            return asString;
        }
        case Roles::ValueTypeRole:
        {
            switch(attribute->valueType())
            {
            case ValueType::Int:
            case ValueType::Float:
                return tr("Numerical");
            case ValueType::String:
                return tr("Textual");
            default: break;
            }

            return tr("Unknown Type");
        }
        default:
            return {};
        }
    }

    return itemvalue;
}

Qt::ItemFlags AvailableAttributesModel::flags(const QModelIndex& index) const
{
    if(!index.isValid())
        return 0;

    return QAbstractItemModel::flags(index);
}

QVariant AvailableAttributesModel::headerData(int /*section*/, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return _root->value();

    return {};
}

QModelIndex AvailableAttributesModel::index(int row, int column, const QModelIndex& parentIndex) const
{
    if(!hasIndex(row, column, parentIndex))
        return {};

    AvailableAttributesModel::Item* parent;

    if(!parentIndex.isValid())
        parent = _root;
    else
        parent = static_cast<AvailableAttributesModel::Item*>(parentIndex.internalPointer());

    auto* child = parent->child(row);
    if(child != nullptr)
        return createIndex(row, column, child);

    return {};
}

AvailableAttributesModel::Item* AvailableAttributesModel::parentItem(const QModelIndex& index) const
{
    if(!index.isValid())
        return nullptr;

    auto* child = static_cast<AvailableAttributesModel::Item*>(index.internalPointer());
    return child->parent();
}

QModelIndex AvailableAttributesModel::parent(const QModelIndex& index) const
{
    auto* parent = parentItem(index);

    if(parent == nullptr || parent == _root)
        return {};

    return createIndex(parent->row(), 0, parent);
}

int AvailableAttributesModel::rowCount(const QModelIndex& parentIndex) const
{
    AvailableAttributesModel::Item* parent;
    if(parentIndex.column() > 0)
        return 0;

    if(!parentIndex.isValid())
        parent = _root;
    else
        parent = static_cast<AvailableAttributesModel::Item*>(parentIndex.internalPointer());

    return parent->childCount();
}

int AvailableAttributesModel::columnCount(const QModelIndex& /*parent*/) const
{
    return 1;
}

QVariant AvailableAttributesModel::get(const QModelIndex& index) const
{
    auto* parent = parentItem(index);

    QString preamble;

    if(parent == _sourceNode)
        preamble = "source.";
    else if(parent == _targetNode)
        preamble = "target.";

    return preamble + data(index, Qt::DisplayRole).toString();
}

QHash<int, QByteArray> AvailableAttributesModel::roleNames() const
{
    auto names = QAbstractItemModel::roleNames();

    names[Roles::ElementTypeRole] = "elementType";
    names[Roles::ValueTypeRole] = "valueType";

    return names;
}

void registerAvailableAttributesModelType()
{
    qmlRegisterInterface<AvailableAttributesModel>("AvailableAttributesModel");
}

Q_COREAPP_STARTUP_FUNCTION(registerAvailableAttributesModelType)
