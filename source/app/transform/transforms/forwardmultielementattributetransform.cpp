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

#include "forwardmultielementattributetransform.h"

#include "transform/transformedgraph.h"
#include "graph/graphmodel.h"

#include "shared/utils/typeidentity.h"
#include "shared/utils/msvcwarningsuppress.h"

#include <memory>

#include <QObject>

static Alert forwardMultiElementAttributeTransformConfigIsValid(const GraphTransformConfig& config)
{
    if(config.attributeNames().empty())
        return {AlertType::Error, QObject::tr("Invalid parameter")};

    return {AlertType::None, {}};
}

template<typename E, typename V>
struct AttributeProxyFunctor
{
    Attribute _proxiedAttribute;
    ElementIdArray<E, E> _headMap;

    V operator()(E elementId)
    {
        // Always use the head ID's value
        auto headElementId = _headMap.at(elementId);
        Q_ASSERT(!headElementId.isNull());
        return _proxiedAttribute.valueOf<V>(headElementId);
    }
};

void ForwardMultiElementAttributeTransform::apply(TransformedGraph& target) const
{
    target.setPhase(QObject::tr("Forward Multi-Element Attribute"));

    auto alert = forwardMultiElementAttributeTransformConfigIsValid(config());
    if(alert._type != AlertType::None)
    {
        addAlert(alert);
        return;
    }

    auto attributeName = config().attributeNames().front();

    // Take a copy of the attribute, and remove it
    auto proxiedAttribute = _graphModel->attributeValueByName(attributeName);
    _graphModel->removeAttribute(attributeName);

    // Clone a new attribute from the copy
    auto& attribute = _graphModel->createAttribute(attributeName);
    attribute = proxiedAttribute;

    auto setProxyFunctor = [&](const auto& elementIds, const auto mergedElementIdsForElementId)
    {
        using E = typename std::remove_reference<decltype(elementIds)>::type::value_type;
        ElementIdArray<E, E> headMap(target);

        // Create a map of merged element IDs to respective head IDs
        for(auto elementId : elementIds)
        {
            switch(target.typeOf(elementId))
            {
            case MultiElementType::Not: headMap[elementId] = elementId; [[fallthrough]];
            case MultiElementType::Tail: continue;
            default: break;
            }

            MSVC_WARNING_SUPPRESS_NEXTLINE(6001)
            auto mergedElementIds = (target.*mergedElementIdsForElementId)(elementId);
            for(auto mergedElementId : mergedElementIds)
                headMap[mergedElementId] = elementId;
        }

        switch(proxiedAttribute.valueType())
        {
        case ValueType::Int:    attribute.setIntValueFn(AttributeProxyFunctor<E, int>{proxiedAttribute, headMap});        break;
        case ValueType::Float:  attribute.setFloatValueFn(AttributeProxyFunctor<E, double>{proxiedAttribute, headMap});   break;
        case ValueType::String: attribute.setStringValueFn(AttributeProxyFunctor<E, QString>{proxiedAttribute, headMap}); break;
        default: Q_ASSERT(!"Unhandled ValueType"); break;
        }

        if(proxiedAttribute.hasMissingValues())
        {
            // Set valueMissingOf function to always reference head
            attribute.setValueMissingFn([proxiedAttribute, headMap](E elementId)
            {
                auto headElementId = headMap.at(elementId);
                Q_ASSERT(!headElementId.isNull());
                return proxiedAttribute.valueMissingOf(headElementId);
            });
        }
    };

    switch(proxiedAttribute.elementType())
    {
    case ElementType::Node: setProxyFunctor(target.nodeIds(), &Graph::mergedNodeIdsForNodeId); break;
    case ElementType::Edge: setProxyFunctor(target.edgeIds(), &Graph::mergedEdgeIdsForEdgeId); break;
    default: Q_ASSERT(!"Unhandled ElementType"); break;
    }
}

bool ForwardMultiElementAttributeTransformFactory::configIsValid(const GraphTransformConfig& graphTransformConfig) const
{
    return forwardMultiElementAttributeTransformConfigIsValid(graphTransformConfig)._type == AlertType::None;
}

std::unique_ptr<GraphTransform> ForwardMultiElementAttributeTransformFactory::create(
    const GraphTransformConfig&) const
{
    return std::make_unique<ForwardMultiElementAttributeTransform>(*graphModel());
}
