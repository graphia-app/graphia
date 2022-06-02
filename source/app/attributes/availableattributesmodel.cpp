/* Copyright © 2013-2022 Graphia Technologies Ltd.
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
#include "shared/utils/static_block.h"

AvailableAttributesModel::Item::Item(const QString& value, const Attribute* attribute) :
    _value(value), _attribute(attribute)
{}

AvailableAttributesModel::Item::~Item() // NOLINT modernize-use-equals-default
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

bool AvailableAttributesModel::Item::hasAncestor(AvailableAttributesModel::Item* potentialAncestor) const
{
    if(this == potentialAncestor)
        return true;

    if(_parent == nullptr)
        return false;

    if(_parent == potentialAncestor)
        return true;

    return _parent->hasAncestor(potentialAncestor);
}

const Attribute* AvailableAttributesModel::Item::attribute() const
{
    return _attribute;
}

AvailableAttributesModel::AvailableAttributesModel(const GraphModel& graphModel,
    QObject* parent, ElementType elementTypes,
    ValueType valueTypes, AttributeFlag skipFlags,
    const QStringList& skipAttributeNames) :
    QAbstractItemModel(parent),
    _graphModel(&graphModel)
{
    _root = new AvailableAttributesModel::Item(tr("Attribute"));

    auto attributeList = graphModel.availableAttributeNames(
        elementTypes, valueTypes, skipFlags, skipAttributeNames);

    auto addItem = [this, &graphModel](Item* parentItem, const QString& name)
    {
        const auto* attribute = graphModel.attributeByName(name);
        Q_ASSERT(attribute != nullptr);

        auto* attributeItem = new AvailableAttributesModel::Item(name, attribute);
        parentItem->addChild(attributeItem);

        if(attribute->hasParameter())
        {
            _attributeItemsWithParameters.emplace(attributeItem);

            const auto& validValues = attribute->validParameterValues();
            for(const auto& validValue : validValues)
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
        attributeList = graphModel.availableAttributeNames(ElementType::Node, valueTypes,
            skipFlags, skipAttributeNames);

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

// NOLINTNEXTLINE modernize-use-equals-default
AvailableAttributesModel::~AvailableAttributesModel()
{
    delete _root;
}

QVariant AvailableAttributesModel::data(const QModelIndex& index, int role) const
{
    if(!index.isValid())
        return {};

    auto* item = itemForIndex(index);

    if(role == Roles::HasChildrenRole)
        return item->childCount() > 0;

    const auto& itemValue = item->value();

    if(role == Qt::DisplayRole)
        return itemValue;

    // Specifically when querying element type, report any descendent of the
    // source or target node items as edge types; bit of a hack
    if(role == Roles::ElementTypeRole && (item->hasAncestor(_sourceNode) || item->hasAncestor(_targetNode)))
        return elementTypeAsString(ElementType::Edge);

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
        static const QString numericalTr = tr("Numerical");
        static const QString textualTr = tr("Textual");
        static const QString unknownTypeTr = tr("Unknown Type");

        switch(attribute->valueType())
        {
        case ValueType::Int:
        case ValueType::Float:
            return numericalTr;
        case ValueType::String:
            return textualTr;
        default: break;
        }

        return unknownTypeTr;
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
        static const QString userDefinedTr = tr("User Defined");
        static const QString calculatedTr = tr("Calculated");

        return attribute->userDefined() ? userDefinedTr : calculatedTr;
    }
    case Roles::EditableRole:
    {
        return attribute->editable();
    }
    case Roles::HasParameterRole:
    {
        return attribute->hasParameter();
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

    auto flags = QAbstractItemModel::flags(index);
    flags.setFlag(Qt::ItemIsSelectable, itemForIndex(index)->childCount() == 0);

    return flags;
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
        parent = itemForIndex(parentIndex);

    auto* child = parent->child(row);
    if(child != nullptr)
        return createIndex(row, column, child);

    return {};
}

AvailableAttributesModel::Item* AvailableAttributesModel::parentItem(const QModelIndex& index)
{
    if(!index.isValid())
        return nullptr;

    return itemForIndex(index)->parent();
}

AvailableAttributesModel::Item* AvailableAttributesModel::itemForIndex(const QModelIndex& index)
{
    auto* item = static_cast<Item*>(index.internalPointer());
    Q_ASSERT(item != nullptr);

    return item;
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
        parent = itemForIndex(parentIndex);

    return parent->childCount();
}

int AvailableAttributesModel::columnCount(const QModelIndex& /*parent*/) const
{
    return 1;
}

QString AvailableAttributesModel::get(const QModelIndex& index) const
{
    if(!index.isValid())
        return {};

    auto* item = itemForIndex(index);
    if(item == nullptr)
        return {};

    QString text = QStringLiteral(R"("%1")").arg(item->value());

    auto* parent = parentItem(index);
    if(u::contains(_attributeItemsWithParameters, parent))
    {
        QString parentText = get(index.parent());
        text = QStringLiteral("%1.%2").arg(parentText, text);
    }
    else if(parent == _sourceNode)
        text = QStringLiteral("source.%1").arg(text);
    else if(parent == _targetNode)
        text = QStringLiteral("target.%1").arg(text);

    return text;
}

QModelIndex AvailableAttributesModel::find(const QString& name) const
{
    auto attributeName = Attribute::parseAttributeName(name);

    AvailableAttributesModel::Item* rootItem = nullptr;

    switch(attributeName._type)
    {
    case Attribute::EdgeNodeType::Source: rootItem = _sourceNode; break;
    case Attribute::EdgeNodeType::Target: rootItem = _targetNode; break;
    default: break;
    }

    QModelIndex rootIndex;

    if(rootItem != nullptr)
    {
        for(int row = 0; row < rowCount(); row++)
        {
            const auto& itemIndex = index(row, 0);

            if(itemForIndex(itemIndex) == rootItem)
            {
                rootIndex = itemIndex;
                break;
            }
        }
    }

    auto indexMatching = [this](const QModelIndex& parentIndex, const QString& value)
    {
        for(int row = 0; row < rowCount(parentIndex); row++)
        {
            const auto& itemIndex = index(row, 0, parentIndex);

            if(itemForIndex(itemIndex)->value() == value)
                return itemIndex;
        }

        return QModelIndex();
    };

    auto matchingIndex = indexMatching(rootIndex, attributeName._name);

    if(!attributeName._parameter.isEmpty())
        matchingIndex = indexMatching(matchingIndex, attributeName._parameter);

    return matchingIndex;
}

QHash<int, QByteArray> AvailableAttributesModel::roleNames() const
{
    auto names = QAbstractItemModel::roleNames();

    names[Roles::ElementTypeRole] = "elementType";
    names[Roles::ValueTypeRole] = "valueType";
    names[Roles::HasSharedValuesRole] = "hasSharedValues";
    names[Roles::SearchableRole] = "searchable";
    names[Roles::UserDefinedRole] = "userDefined";
    names[Roles::EditableRole] = "editable";
    names[Roles::HasChildrenRole] = "hasChildren";
    names[Roles::HasParameterRole] = "hasParameter";

    return names;
}

static_block
{
    qmlRegisterInterface<AvailableAttributesModel>("AvailableAttributesModel"
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        , Application::majorVersion()
#endif
        );
}
