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

#include "conditionfncreator.h"

bool conditionIsValid(ElementType elementType, const GraphModel& graphModel,
                      const GraphTransformConfig::Condition& condition)
{
    switch(elementType)
    {
    case ElementType::Node:         return CreateConditionFnFor::node(graphModel, condition)      != nullptr;
    case ElementType::Edge:         return CreateConditionFnFor::edge(graphModel, condition)      != nullptr;
    case ElementType::Component:    return CreateConditionFnFor::component(graphModel, condition) != nullptr;
    default:                        return false;
    }

    return false;
}
