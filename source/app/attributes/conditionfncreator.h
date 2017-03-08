#ifndef CONDITIONFNCREATOR_H
#define CONDITIONFNCREATOR_H

#include "condtionfnops.h"

#include "shared/graph/elementid.h"
#include "shared/graph/igraphcomponent.h"
#include "shared/utils/utils.h"
#include "datafield.h"

#include "thirdparty/boost/boost_disable_warnings.h"
#include "boost/variant/static_visitor.hpp"
#include "thirdparty/boost/boost_enable_warnings.h"

class CreateConditionFnFor
{
private:
    template<typename E>
    struct OpValueVisitor : public boost::static_visitor<ElementConditionFn<E>>
    {
        const DataField* _dataField;

        OpValueVisitor(const DataField* dataField) :
            _dataField(dataField)
        {}

        template<typename T>
        ElementConditionFn<E> numericalFn(FieldType type, ConditionFnOp::Numerical op, T value) const
        {
            const auto* dataField = _dataField;

            if(dataField->valueType() != type)
                return nullptr;

            // Clamp the value if necessary
            switch(dataField->valueType())
            {
            case FieldType::Int:
                if(dataField->hasIntMin() && value < dataField->intMin()) value = dataField->intMin();
                if(dataField->hasIntMax() && value > dataField->intMax()) value = dataField->intMax();
                break;

            case FieldType::Float:
                if(dataField->hasFloatMin() && value < dataField->floatMin()) value = dataField->floatMin();
                if(dataField->hasFloatMax() && value > dataField->floatMax()) value = dataField->floatMax();
                break;

            default: break;
            }

            switch(op)
            {
            case ConditionFnOp::Numerical::Equal:
                return [dataField, value](E elementId) { return dataField->template valueOf<T, E>(elementId) == value; };
            case ConditionFnOp::Numerical::NotEqual:
                return [dataField, value](E elementId) { return dataField->template valueOf<T, E>(elementId) != value; };
            case ConditionFnOp::Numerical::LessThan:
                return [dataField, value](E elementId) { return dataField->template valueOf<T, E>(elementId) < value; };
            case ConditionFnOp::Numerical::GreaterThan:
                return [dataField, value](E elementId) { return dataField->template valueOf<T, E>(elementId) > value; };
            case ConditionFnOp::Numerical::LessThanOrEqual:
                return [dataField, value](E elementId) { return dataField->template valueOf<T, E>(elementId) <= value; };
            case ConditionFnOp::Numerical::GreaterThanOrEqual:
                return [dataField, value](E elementId) { return dataField->template valueOf<T, E>(elementId) >= value; };
            default:
                qFatal("Unhandled NumericalOp");
                return nullptr;
            }
        }

        ElementConditionFn<E> stringFn(ConditionFnOp::String op, QString value) const
        {
            const auto* dataField = _dataField;

            DataField::ValueOfFn<QString, E> valueOfFn = &DataField::valueOf<QString, E>;

            // If we don't have a string DataField, then use the ValueOfFn that converts
            // the DataField's value to a string
            if(dataField->valueType() != FieldType::String)
                valueOfFn = &DataField::stringValueOf<E>;

            switch(op)
            {
            case ConditionFnOp::String::Equal:
                return [dataField, valueOfFn, value](E elementId) { return (dataField->*valueOfFn)(elementId) == value; };
            case ConditionFnOp::String::NotEqual:
                return [dataField, valueOfFn, value](E elementId) { return (dataField->*valueOfFn)(elementId) != value; };
            case ConditionFnOp::String::Includes:
                return [dataField, valueOfFn, value](E elementId) { return (dataField->*valueOfFn)(elementId).contains(value); };
            case ConditionFnOp::String::Excludes:
                return [dataField, valueOfFn, value](E elementId) { return !(dataField->*valueOfFn)(elementId).contains(value); };
            case ConditionFnOp::String::Starts:
                return [dataField, valueOfFn, value](E elementId) { return (dataField->*valueOfFn)(elementId).startsWith(value); };
            case ConditionFnOp::String::Ends:
                return [dataField, valueOfFn, value](E elementId) { return (dataField->*valueOfFn)(elementId).endsWith(value); };
            case ConditionFnOp::String::MatchesRegex:
            {
                QRegularExpression re(value);
                if(!re.isValid())
                    return nullptr; // Regex isn't valid

                return [dataField, valueOfFn, re](E elementId) { return re.match((dataField->*valueOfFn)(elementId)).hasMatch(); };
            }
            default:
                qFatal("Unhandled StringOp");
                return nullptr;
            }
        }

        ElementConditionFn<E> operator()(const GraphTransformConfig::IntOpValue& opValue) const
        { return numericalFn(FieldType::Int, opValue._op, opValue._value); }
        ElementConditionFn<E> operator()(const GraphTransformConfig::FloatOpValue& opValue) const
        { return numericalFn(FieldType::Float, opValue._op, opValue._value); }
        ElementConditionFn<E> operator()(const GraphTransformConfig::StringOpValue& opValue) const
        { return stringFn(opValue._op, opValue._value); }
    };

    template<typename E>
    struct ConditionVisitor : public boost::static_visitor<ElementConditionFn<E>>
    {
        ElementType _elementType;
        const std::map<QString, DataField>* _dataFields;

        ConditionVisitor(ElementType elementType, const std::map<QString, DataField>& dataFields) :
            _elementType(elementType),
            _dataFields(&dataFields)
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
            const auto& fieldName = terminalCondition._field;

            if(!u::contains(*_dataFields, fieldName))
                return nullptr; // Unknown field

            const auto& dataField = _dataFields->at(fieldName);

            if(dataField.elementType() != _elementType)
                return nullptr; // Mismatched elementTypes

            return boost::apply_visitor(OpValueVisitor<E>(&dataField), terminalCondition._opValue);
        }

        ElementConditionFn<E> operator()(const GraphTransformConfig::CompoundCondition& compoundCondition) const
        {
            auto lhs = boost::apply_visitor(ConditionVisitor<E>(_elementType, *_dataFields), compoundCondition._lhs);
            auto rhs = boost::apply_visitor(ConditionVisitor<E>(_elementType, *_dataFields), compoundCondition._rhs);

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
    static auto node(const std::map<QString, DataField>& dataFields,
                     const GraphTransformConfig::Condition& condition)
    {
        return boost::apply_visitor(ConditionVisitor<NodeId>(ElementType::Node, dataFields), condition);
    }

    static auto edge(const std::map<QString, DataField>& dataFields,
                     const GraphTransformConfig::Condition& condition)
    {
        return boost::apply_visitor(ConditionVisitor<EdgeId>(ElementType::Edge, dataFields), condition);
    }

    static auto component(const std::map<QString, DataField>& dataFields,
                          const GraphTransformConfig::Condition& condition)
    {
        return boost::apply_visitor(ConditionVisitor<const IGraphComponent&>(ElementType::Component, dataFields), condition);
    }

    template<typename Op, typename Value>
    static auto node(const DataField& dataField, Op op, Value value)
    {
        GraphTransformConfig::OpValue opValue = createOpValue(op, value);
        return boost::apply_visitor(OpValueVisitor<NodeId>(&dataField), opValue);
    }

    template<typename Op, typename Value>
    static auto edge(const DataField& dataField, Op op, Value value)
    {
        GraphTransformConfig::OpValue opValue = createOpValue(op, value);
        return boost::apply_visitor(OpValueVisitor<EdgeId>(&dataField), opValue);
    }

    template<typename Op, typename Value>
    static auto component(const DataField& dataField, Op op, Value value)
    {
        GraphTransformConfig::OpValue opValue = createOpValue(op, value);
        return boost::apply_visitor(OpValueVisitor<const IGraphComponent&>(&dataField), opValue);
    }
};

#endif // CONDITIONFNCREATOR_H
