#include "conditionfncreator.h"

bool conditionIsValid(ElementType elementType, const NameAttributeMap& attributes,
                      const GraphTransformConfig::Condition& condition)
{
    switch(elementType)
    {
    case ElementType::Node:         return CreateConditionFnFor::node(attributes, condition)      != nullptr;
    case ElementType::Edge:         return CreateConditionFnFor::edge(attributes, condition)      != nullptr;
    case ElementType::Component:    return CreateConditionFnFor::component(attributes, condition) != nullptr;
    default:                        return false;
    }

    return false;
}
