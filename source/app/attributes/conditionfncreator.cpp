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
