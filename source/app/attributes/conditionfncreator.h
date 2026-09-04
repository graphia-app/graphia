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

#ifndef CONDITIONFNCREATOR_H
#define CONDITIONFNCREATOR_H

#include "shared/graph/elementid.h"
#include "shared/graph/elementid_containers.h"
#include "shared/graph/elementtype.h"

#include "app/transform/graphtransformconfig.h"

#include <type_traits>

class Attribute;
class GraphModel;
class IGraphComponent;

class CreateConditionFnFor
{
public:
    static NodeConditionFn node(const GraphModel& graphModel,
        const GraphTransformConfig::Condition& condition);
    static EdgeConditionFn edge(const GraphModel& graphModel,
        const GraphTransformConfig::Condition& condition);
    static ComponentConditionFn component(const GraphModel& graphModel,
        const GraphTransformConfig::Condition& condition);

    template<typename E>
    static auto elementType(const GraphModel& graphModel,
        const GraphTransformConfig::Condition& condition)
    {
        if constexpr(std::is_same_v<E, NodeId>)
            return node(graphModel, condition);

        if constexpr(std::is_same_v<E, EdgeId>)
            return edge(graphModel, condition);

        if constexpr(std::is_same_v<E, const IGraphComponent&>)
            return component(graphModel, condition);
    }

    static NodeConditionFn node(const Attribute& attribute,
        const GraphTransformConfig::TerminalOp& op,
        const GraphTransformConfig::TerminalValue& value);
    static EdgeConditionFn edge(const Attribute& attribute,
        const GraphTransformConfig::TerminalOp& op,
        const GraphTransformConfig::TerminalValue& value);
    static ComponentConditionFn component(const Attribute& attribute,
        const GraphTransformConfig::TerminalOp& op,
        const GraphTransformConfig::TerminalValue& value);

    template<typename E>
    static auto elementType(const Attribute& attribute,
        const GraphTransformConfig::TerminalOp& op,
        const GraphTransformConfig::TerminalValue& value)
    {
        if constexpr(std::is_same_v<E, NodeId>)
            return node(attribute, op, value);

        if constexpr(std::is_same_v<E, EdgeId>)
            return edge(attribute, op, value);

        if constexpr(std::is_same_v<E, const IGraphComponent&>)
            return component(attribute, op, value);
    }
};

bool conditionIsValid(ElementType elementType, const GraphModel& graphModel,
                      const GraphTransformConfig::Condition& condition);

#endif // CONDITIONFNCREATOR_H
