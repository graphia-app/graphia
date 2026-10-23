/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

#include "cloneattributecommand.h"

#include "app/graph/graphmodel.h"
#include "app/graph/graph.h"
#include "app/graph/mutablegraph.h"

#include "shared/utils/string.h"

#include "shared/loading/userelementdata.h"

#include <QLocale>
#include <QObject>
#include <QtGlobal>

#include <type_traits>

using namespace Qt::Literals::StringLiterals;

CloneAttributeCommand::CloneAttributeCommand(GraphModel* graphModel, const QString& sourceAttributeName,
    const QString& newAttributeName) :
    _graphModel(graphModel), _sourceAttributeName(sourceAttributeName), _newAttributeName(newAttributeName)
{}

QString CloneAttributeCommand::description() const
{
    return QObject::tr("Clone Attribute");
}

QString CloneAttributeCommand::verb() const
{
    return QObject::tr("Cloning Attribute");
}

QString CloneAttributeCommand::pastParticiple() const
{
    return QObject::tr("Attribute %1 Cloned").arg(_graphModel->attributeDisplayName(_sourceAttributeName));
}

QString CloneAttributeCommand::debugDescription() const
{
    QString text = description();

    text.append(u"\n  %1"_s.arg(_createdAttributeNames.front()));

    return text;
}

bool CloneAttributeCommand::execute()
{
    const AttributeChangesTracker tracker(_graphModel);

    auto sourceAttribute = _graphModel->attributeValueByName(_sourceAttributeName);
    Q_ASSERT(sourceAttribute.isValid());

    auto createAttribute = [&](const auto& elementIds)
    {
        using E = typename std::remove_reference_t<decltype(elementIds)>::value_type;

        UserElementData<E>* userData = nullptr;

        if constexpr(std::is_same_v<E, NodeId>)
            userData = &_graphModel->userNodeData();
        else if constexpr(std::is_same_v<E, EdgeId>)
            userData = &_graphModel->userEdgeData();

        _createdVectorName = u::findUniqueName(userData->vectorNames(), _newAttributeName);

        for(auto elementId : elementIds)
        {
            // Attribute::stringValueOf limits the precision of floating point values
            QString value;
            if(sourceAttribute.valueType() == ValueType::Float)
            {
                value = QString::number(sourceAttribute.floatValueOf(elementId),
                    'g', QLocale::FloatingPointShortest);
            }
            else
                value = sourceAttribute.stringValueOf(elementId);

            if(sourceAttribute.valueMissingOf(elementId))
                value.clear();

            userData->setValueBy(elementId, _createdVectorName, value);
        }

        return userData->exposeAsAttributes(*_graphModel);
    };

    // Attributes from the user's data have values for every element, including those that
    // transforms have removed or merged, whereas computed attributes only have values for
    // the elements of the transformed graph
    const Graph& graph = sourceAttribute.userDefined() ?
        static_cast<const Graph&>(_graphModel->mutableGraph()) : _graphModel->graph();

    if(sourceAttribute.elementType() == ElementType::Node)
        _createdAttributeNames = createAttribute(graph.nodeIds());
    else if(sourceAttribute.elementType() == ElementType::Edge)
        _createdAttributeNames = createAttribute(graph.edgeIds());

    return !_createdAttributeNames.empty();
}

void CloneAttributeCommand::undo()
{
    const AttributeChangesTracker tracker(_graphModel);

    for(const auto& attributeName : _createdAttributeNames)
        _graphModel->removeAttribute(attributeName);

    _createdAttributeNames.clear();

    const auto* sourceAttribute = _graphModel->attributeByName(_sourceAttributeName);
    Q_ASSERT(sourceAttribute != nullptr);

    if(sourceAttribute->elementType() == ElementType::Node)
        _graphModel->userNodeData().remove(_createdVectorName);
    else if(sourceAttribute->elementType() == ElementType::Edge)
        _graphModel->userEdgeData().remove(_createdVectorName);

    _createdVectorName.clear();
}
