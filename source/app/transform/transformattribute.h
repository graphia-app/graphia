/* Copyright © 2013-2021 Graphia Technologies Ltd.
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

#ifndef TRANSFORMATTRIBUTE_H
#define TRANSFORMATTRIBUTE_H

#include "attributes/attribute.h"

#include "shared/attributes/valuetype.h"
#include "shared/graph/elementtype.h"
#include "shared/graph/elementid.h"
#include "shared/graph/grapharray.h"

#include "graph/graphmodel.h"

#include "transform/transformedgraph.h"

#include <QString>
#include <QtGlobal>

template<typename E, typename V, typename ValueOfFn>
struct AttributeProxyFunctor
{
    Attribute _proxiedAttribute;
    ElementIdArray<E, E> _headMap;
    ValueOfFn _valueOfFn;

    V operator()(E elementId)
    {
        auto proxiedValueOfFn = [this](E elementId2)
        {
            return _proxiedAttribute.valueOf<V>(elementId2);
        };

        auto headElementId = _headMap.at(elementId);
        Q_ASSERT(!headElementId.isNull());

        return _valueOfFn(proxiedValueOfFn, headElementId, elementId);
    }
};

template<typename ValueOfFn, typename ValueMissingOfFn>
bool transformAttribute(GraphModel* graphModel, TransformedGraph& target, const QString& attributeName,
    const ValueOfFn& valueOfFn, const ValueMissingOfFn& valueMissingOfFn)
{
    if(!graphModel->attributeExists(attributeName))
        return false;

    // Take a copy of the attribute, and remove it
    auto proxiedAttribute = graphModel->attributeValueByName(attributeName);
    graphModel->removeAttribute(attributeName);

    // Clone a new attribute from the copy
    auto& attribute = graphModel->createAttribute(attributeName);
    attribute = proxiedAttribute;

    auto setProxyFn = [&](const auto& elementIds, const auto mergedElementIdsForElementId)
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

            auto mergedElementIds = (target.*mergedElementIdsForElementId)(elementId);
            for(auto mergedElementId : mergedElementIds)
                headMap[mergedElementId] = elementId;
        }

        switch(proxiedAttribute.valueType())
        {
        case ValueType::Int:    attribute.setIntValueFn(AttributeProxyFunctor<E, int, decltype(valueOfFn)>{proxiedAttribute, headMap, valueOfFn});        break;
        case ValueType::Float:  attribute.setFloatValueFn(AttributeProxyFunctor<E, double, decltype(valueOfFn)>{proxiedAttribute, headMap, valueOfFn});   break;
        case ValueType::String: attribute.setStringValueFn(AttributeProxyFunctor<E, QString, decltype(valueOfFn)>{proxiedAttribute, headMap, valueOfFn}); break;
        default: Q_ASSERT(!"Unhandled ValueType"); break;
        }

        if(proxiedAttribute.hasMissingValues())
        {
            attribute.setValueMissingFn([proxiedAttribute, headMap, valueMissingOfFn](E elementId)
            {
                auto proxiedValueMissingOfFn = [&proxiedAttribute](E elementId2)
                {
                    return proxiedAttribute.valueMissingOf(elementId2);
                };

                auto headElementId = headMap.at(elementId);
                Q_ASSERT(!headElementId.isNull());

                return valueMissingOfFn(proxiedValueMissingOfFn, headElementId, elementId);
            });
        }
    };

    switch(proxiedAttribute.elementType())
    {
    case ElementType::Node: setProxyFn(target.nodeIds(), &Graph::mergedNodeIdsForNodeId); break;
    case ElementType::Edge: setProxyFn(target.edgeIds(), &Graph::mergedEdgeIdsForEdgeId); break;
    default: Q_ASSERT(!"Unhandled ElementType"); break;
    }

    return true;
}

#endif // TRANSFORMATTRIBUTE_H
