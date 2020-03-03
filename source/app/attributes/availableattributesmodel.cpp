#include "availableattributesmodel.h"

#include "application.h"

#include "../crashhandler.h"

#include "graph/graphmodel.h"

#include "shared/graph/elementtype.h"
#include "shared/utils/container.h"

AvailableAttributesModel::Item::Item(QVariant value) :
    _value(std::move(value))
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
        return _parent->_children.indexOf(const_cast<Item*>(this)); // NOLINT

    return 0;
}

AvailableAttributesModel::Item*AvailableAttributesModel::Item::parent()
{
    return _parent;
}

AvailableAttributesModel::AvailableAttributesModel(const GraphModel& graphModel,
                                                   QObject* parent,
                                                   ElementType elementTypes,
                                                   ValueType valueTypes,
                                                   AttributeFlag skipFlags) :
    QAbstractItemModel(parent),
    _graphModel(&graphModel)
{
    _root = new AvailableAttributesModel::Item(tr("Attribute"));

    auto attributeList = graphModel.availableAttributeNames(elementTypes, valueTypes, skipFlags);

    auto addItem = [this, &graphModel](Item* parentItem, const QString& name)
    {
        auto* attributeItem = new AvailableAttributesModel::Item(name);
        parentItem->addChild(attributeItem);

        const auto* attribute = graphModel.attributeByName(name);
        Q_ASSERT(attribute != nullptr);

        if(attribute->hasParameter())
        {
            _attributeItemsWithParameters.push_back(attributeItem);

            for(const auto& validValue : attribute->validParameterValues())
            {
                auto* parameterItem = new AvailableAttributesModel::Item(validValue);
                attributeItem->addChild(parameterItem);
            }
        }
    };

    for(const auto& attributeName : qAsConst(attributeList))
        addItem(_root, attributeName);

    if(Flags<ElementType>(elementTypes).test(ElementType::Edge))
    {
        attributeList = graphModel.availableAttributeNames(ElementType::Node, valueTypes);

        if(!attributeList.empty())
        {
            _sourceNode = new AvailableAttributesModel::Item(tr("Source Node"));
            _targetNode = new AvailableAttributesModel::Item(tr("Target Node"));
            _root->addChild(_sourceNode);
            _root->addChild(_targetNode);

            for(const auto& attributeName : qAsConst(attributeList))
            {
                addItem(_sourceNode, attributeName);
                addItem(_targetNode, attributeName);
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

    auto itemValue = item->value().toString();

    if(role == Qt::DisplayRole)
        return itemValue;

    // Special case for source/target node items, which don't have sections
    if(item->childCount() > 0)
        return {};

    const auto* attribute = _graphModel->attributeByName(itemValue);

    if(attribute == nullptr || !attribute->isValid())
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
    case Roles::HasSharedValuesRole:
    {
        return !attribute->sharedValues().empty();
    }
    case Roles::SearchableRole:
    {
        return attribute->testFlag(AttributeFlag::Searchable);
    }
    case Roles::UserDefinedRole:
    {
        return attribute->userDefined() ? tr("User Defined") : tr("Calculated");
    }
    default:
        return {};
    }

    return {};
}

Qt::ItemFlags AvailableAttributesModel::flags(const QModelIndex& index) const
{
    if(!index.isValid())
        return 0; // NOLINT modernize-use-nullptr, huh?

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

    AvailableAttributesModel::Item* parent = nullptr;

    if(!parentIndex.isValid())
        parent = _root;
    else
        parent = static_cast<AvailableAttributesModel::Item*>(parentIndex.internalPointer());

    auto* child = parent->child(row);
    if(child != nullptr)
        return createIndex(row, column, child);

    return {};
}

AvailableAttributesModel::Item* AvailableAttributesModel::parentItem(const QModelIndex& index)
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
    AvailableAttributesModel::Item* parent = nullptr;
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
    QString text = QStringLiteral("\"%1\"").arg(data(index, Qt::DisplayRole).toString());

    if(u::contains(_attributeItemsWithParameters, parent))
    {
        QString parentText = get(index.parent()).toString();
        text = QStringLiteral("%1.%2").arg(parentText, text);
    }
    else if(parent == _sourceNode)
        text = QStringLiteral("source.%1").arg(text);
    else if(parent == _targetNode)
        text = QStringLiteral("target.%1").arg(text);

    return text;
}

QHash<int, QByteArray> AvailableAttributesModel::roleNames() const
{
    auto names = QAbstractItemModel::roleNames();

    names[Roles::ElementTypeRole] = "elementType";
    names[Roles::ValueTypeRole] = "valueType";
    names[Roles::HasSharedValuesRole] = "hasSharedValues";
    names[Roles::SearchableRole] = "searchable";
    names[Roles::UserDefinedRole] = "userDefined";

    return names;
}

void registerAvailableAttributesModelType()
{
    qmlRegisterInterface<AvailableAttributesModel>("AvailableAttributesModel");
}

Q_COREAPP_STARTUP_FUNCTION(registerAvailableAttributesModelType)
