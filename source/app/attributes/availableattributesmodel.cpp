/* Copyright © 2013-2020 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "availableattributesmodel.h"

#include "application.h"

#include "../crashhandler.h"

#include "graph/graphmodel.h"

#include "shared/graph/elementtype.h"
#include "shared/utils/container.h"

AvailableAttributesModel::Item::Item(QString value, const Attribute* attribute) :
    _value(std::move(value)), _attribute(attribute)
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

AvailableAttributesModel::Item* AvailableAttributesModel::Item::child(int row) const
{
    return _children.value(row);
}

int AvailableAttributesModel::Item::childCount() const
{
    return _children.count();
}

const QString& AvailableAttributesModel::Item::value() const
{
    return _value;
}

int AvailableAttributesModel::Item::row() const
{
    if(_parent != nullptr)
        return _parent->_children.indexOf(const_cast<Item*>(this)); // NOLINT

    return 0;
}

AvailableAttributesModel::Item*AvailableAttributesModel::Item::parent() const
{
    return _parent;
}

const Attribute* AvailableAttributesModel::Item::attribute() const
{
    return _attribute;
}

AvailableAttributesModel::AvailableAttributesModel(const GraphModel& graphModel,
    QObject* parent, ElementType elementTypes,
    ValueType valueTypes, AttributeFlag skipFlags) :
    QAbstractItemModel(parent),
    _graphModel(&graphModel)
{
    _root = new AvailableAttributesModel::Item(tr("Attribute"));

    auto attributeList = graphModel.availableAttributeNames(elementTypes, valueTypes, skipFlags);

    auto addItem = [this, &graphModel](Item* parentItem, const QString& name)
    {
        const auto* attribute = graphModel.attributeByName(name);

        auto* attributeItem = new AvailableAttributesModel::Item(name, attribute);
        parentItem->addChild(attributeItem);

        Q_ASSERT(attribute != nullptr);

        if(attribute->hasParameter())
        {
            _attributeItemsWithParameters.emplace(attributeItem);

            for(const auto& validValue : attribute->validParameterValues())
            {
                auto* parameterItem = new AvailableAttributesModel::Item(validValue, attribute);
                attributeItem->addChild(parameterItem);
            }
        }
    };

    for(const auto& attributeName : std::as_const(attributeList))
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

            for(const auto& attributeName : std::as_const(attributeList))
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

    if(role == Roles::HasChildrenRole)
        return item->childCount() > 0;

    const auto& itemValue = item->value();

    if(role == Qt::DisplayRole)
        return itemValue;

    // Special case for source/target node items, which don't have sections
    if(item->childCount() > 0)
        return {};

    const auto* attribute = item->attribute();

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
        return Qt::NoItemFlags;

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

QString AvailableAttributesModel::get(const QModelIndex& index, int depth) const
{
    auto* item = static_cast<AvailableAttributesModel::Item*>(index.internalPointer());

    if(item == nullptr)
        return {};

    if(item->childCount() > 0 && depth == 0)
        return {};

    QString text = QStringLiteral("\"%1\"").arg(item->value());

    auto* parent = parentItem(index);
    if(u::contains(_attributeItemsWithParameters, parent))
    {
        QString parentText = get(index.parent(), depth + 1);
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
    names[Roles::HasChildrenRole] = "hasChildren";

    return names;
}

void registerAvailableAttributesModelType()
{
    qmlRegisterInterface<AvailableAttributesModel>("AvailableAttributesModel"
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        , Application::majorVersion()
#endif
        );
}

Q_COREAPP_STARTUP_FUNCTION(registerAvailableAttributesModelType)
