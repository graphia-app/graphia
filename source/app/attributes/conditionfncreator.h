#ifndef CONDITIONFNCREATOR_H
#define CONDITIONFNCREATOR_H

#include "condtionfnops.h"

#include "shared/graph/elementid.h"
#include "shared/graph/igraphcomponent.h"
#include "shared/utils/utils.h"
#include "attribute.h"

#include "thirdparty/boost/boost_disable_warnings.h"
#include "boost/variant/static_visitor.hpp"
#include "thirdparty/boost/boost_enable_warnings.h"

class CreateConditionFnFor
{
private:
    template<typename E>
    struct OpValueVisitor : public boost::static_visitor<ElementConditionFn<E>>
    {
        const Attribute* _attribute;

        OpValueVisitor(const Attribute* attribute) :
            _attribute(attribute)
        {}

        template<typename T>
        ElementConditionFn<E> numericalFn(ValueType type, ConditionFnOp::Numerical op, T value) const
        {
            const auto* attribute = _attribute;

            if(attribute->valueType() != type)
                return nullptr;

            switch(op)
            {
            case ConditionFnOp::Numerical::Equal:
                return [attribute, value](E elementId) { return attribute->template valueOf<T, E>(elementId) == value; };
            case ConditionFnOp::Numerical::NotEqual:
                return [attribute, value](E elementId) { return attribute->template valueOf<T, E>(elementId) != value; };
            case ConditionFnOp::Numerical::LessThan:
                return [attribute, value](E elementId) { return attribute->template valueOf<T, E>(elementId) < value; };
            case ConditionFnOp::Numerical::GreaterThan:
                return [attribute, value](E elementId) { return attribute->template valueOf<T, E>(elementId) > value; };
            case ConditionFnOp::Numerical::LessThanOrEqual:
                return [attribute, value](E elementId) { return attribute->template valueOf<T, E>(elementId) <= value; };
            case ConditionFnOp::Numerical::GreaterThanOrEqual:
                return [attribute, value](E elementId) { return attribute->template valueOf<T, E>(elementId) >= value; };
            default:
                qFatal("Unhandled NumericalOp");
                return nullptr;
            }
        }

        ElementConditionFn<E> stringFn(ConditionFnOp::String op, QString value) const
        {
            const auto* attribute = _attribute;

            Attribute::ValueOfFn<QString, E> valueOfFn = &Attribute::valueOf<QString, E>;

            // If we don't have a string Attribute, then use the ValueOfFn that converts
            // the Attribute's value to a string
            if(attribute->valueType() != ValueType::String)
                valueOfFn = &Attribute::stringValueOf<E>;

            switch(op)
            {
            case ConditionFnOp::String::Equal:
                return [attribute, valueOfFn, value](E elementId) { return (attribute->*valueOfFn)(elementId) == value; };
            case ConditionFnOp::String::NotEqual:
                return [attribute, valueOfFn, value](E elementId) { return (attribute->*valueOfFn)(elementId) != value; };
            case ConditionFnOp::String::Includes:
                return [attribute, valueOfFn, value](E elementId) { return (attribute->*valueOfFn)(elementId).contains(value); };
            case ConditionFnOp::String::Excludes:
                return [attribute, valueOfFn, value](E elementId) { return !(attribute->*valueOfFn)(elementId).contains(value); };
            case ConditionFnOp::String::Starts:
                return [attribute, valueOfFn, value](E elementId) { return (attribute->*valueOfFn)(elementId).startsWith(value); };
            case ConditionFnOp::String::Ends:
                return [attribute, valueOfFn, value](E elementId) { return (attribute->*valueOfFn)(elementId).endsWith(value); };
            case ConditionFnOp::String::MatchesRegex:
            {
                QRegularExpression re(value);
                if(!re.isValid())
                    return nullptr; // Regex isn't valid

                return [attribute, valueOfFn, re](E elementId) { return re.match((attribute->*valueOfFn)(elementId)).hasMatch(); };
            }
            default:
                qFatal("Unhandled StringOp");
                return nullptr;
            }
        }

        ElementConditionFn<E> operator()(const GraphTransformConfig::IntOpValue& opValue) const
        { return numericalFn(ValueType::Int, opValue._op, opValue._value); }
        ElementConditionFn<E> operator()(const GraphTransformConfig::FloatOpValue& opValue) const
        { return numericalFn(ValueType::Float, opValue._op, opValue._value); }
        ElementConditionFn<E> operator()(const GraphTransformConfig::StringOpValue& opValue) const
        { return stringFn(opValue._op, opValue._value); }
    };

    template<typename E>
    struct ConditionVisitor : public boost::static_visitor<ElementConditionFn<E>>
    {
        ElementType _elementType;
        const std::map<QString, Attribute>* _attributes;

        ConditionVisitor(ElementType elementType, const std::map<QString, Attribute>& attributes) :
            _elementType(elementType),
            _attributes(&attributes)
        {}

        ElementConditionFn<E> compoundConditionFn(const ElementConditionFn<E>& lhs,
                                                  const ConditionFnOp::Binary& op,
                                                  const ElementConditionFn<E>& rhs) const
        {
            switch(op)
            {
            case ConditionFnOp::Binary::And:
                return [lhs, rhs](E elementId) { return lhs(elementId) && rhs(elementId); };
            case ConditionFnOp::Binary::Or:
                return [lhs, rhs](E elementId) { return lhs(elementId) || rhs(elementId); };
            default:
                qFatal("Unhandled BinaryOp");
                return nullptr;
            }
        }

        ElementConditionFn<E> operator()(const GraphTransformConfig::TerminalCondition& terminalCondition) const
        {
            const auto& attributeName = terminalCondition._attributeName;

            if(!u::contains(*_attributes, attributeName))
                return nullptr; // Unknown attribute

            const auto& attribute = _attributes->at(attributeName);

            if(attribute.elementType() != _elementType)
                return nullptr; // Mismatched elementTypes

            return boost::apply_visitor(OpValueVisitor<E>(&attribute), terminalCondition._opValue);
        }

        ElementConditionFn<E> operator()(const GraphTransformConfig::CompoundCondition& compoundCondition) const
        {
            auto lhs = boost::apply_visitor(ConditionVisitor<E>(_elementType, *_attributes), compoundCondition._lhs);
            auto rhs = boost::apply_visitor(ConditionVisitor<E>(_elementType, *_attributes), compoundCondition._rhs);

            if(lhs == nullptr || rhs == nullptr)
                return nullptr;

            return compoundConditionFn(lhs, compoundCondition._op, rhs);
        }
    };

    static GraphTransformConfig::OpValue createOpValue(ConditionFnOp::Numerical op, int value)
    {
        GraphTransformConfig::IntOpValue opValue = {op, value}; return GraphTransformConfig::OpValue(opValue);
    }

    static GraphTransformConfig::OpValue createOpValue(ConditionFnOp::Numerical op, double value)
    {
        GraphTransformConfig::FloatOpValue opValue = {op, value}; return GraphTransformConfig::OpValue(opValue);
    }

    static GraphTransformConfig::OpValue createOpValue(ConditionFnOp::String op, const QString& value)
    {
        GraphTransformConfig::StringOpValue opValue = {op, value}; return GraphTransformConfig::OpValue(opValue);
    }

public:
    static auto node(const std::map<QString, Attribute>& attributes,
                     const GraphTransformConfig::Condition& condition)
    {
        return boost::apply_visitor(ConditionVisitor<NodeId>(ElementType::Node, attributes), condition);
    }

    static auto edge(const std::map<QString, Attribute>& attributes,
                     const GraphTransformConfig::Condition& condition)
    {
        return boost::apply_visitor(ConditionVisitor<EdgeId>(ElementType::Edge, attributes), condition);
    }

    static auto component(const std::map<QString, Attribute>& attributes,
                          const GraphTransformConfig::Condition& condition)
    {
        return boost::apply_visitor(ConditionVisitor<const IGraphComponent&>(ElementType::Component, attributes), condition);
    }

    template<typename Op, typename Value>
    static auto node(const Attribute& attribute, Op op, Value value)
    {
        GraphTransformConfig::OpValue opValue = createOpValue(op, value);
        return boost::apply_visitor(OpValueVisitor<NodeId>(&attribute), opValue);
    }

    template<typename Op, typename Value>
    static auto edge(const Attribute& attribute, Op op, Value value)
    {
        GraphTransformConfig::OpValue opValue = createOpValue(op, value);
        return boost::apply_visitor(OpValueVisitor<EdgeId>(&attribute), opValue);
    }

    template<typename Op, typename Value>
    static auto component(const Attribute& attribute, Op op, Value value)
    {
        GraphTransformConfig::OpValue opValue = createOpValue(op, value);
        return boost::apply_visitor(OpValueVisitor<const IGraphComponent&>(&attribute), opValue);
    }
};

#endif // CONDITIONFNCREATOR_H
