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

#ifndef CONDITIONFNCREATOR_H
#define CONDITIONFNCREATOR_H

#include "condtionfnops.h"

#include "shared/graph/elementid.h"
#include "shared/graph/igraphcomponent.h"
#include "shared/graph/elementid_containers.h"

#include "graph/graphmodel.h"

#include "transform/graphtransformconfig.h"
#include "transform/graphtransformconfigparser.h"
#include "attribute.h"

#include <boost/variant/static_visitor.hpp>

#include <algorithm>

#include <QRegularExpression>

class CreateConditionFnFor
{
private:
    // Helper to make dealing with value variants a bit easier
    class TerminalValueWrapper
    {
    private:
        GraphTransformConfig::TerminalValue _terminalValue;

    public:
        // cppcheck-suppress noExplicitConstructor
        TerminalValueWrapper(GraphTransformConfig::TerminalValue terminalValue) : // NOLINT
            _terminalValue(std::move(terminalValue))
        {}

        QString toString() const
        {
            struct Visitor
            {
                QString operator()(double v) const          { return QString::number(v); }
                QString operator()(int v) const             { return QString::number(v); }
                QString operator()(const QString& v) const  { return v; }
            };

            return std::visit(Visitor(), _terminalValue);
        }

        double toDouble() const
        {
            struct Visitor
            {
                double operator()(double v) const          { return v; }
                double operator()(int v) const             { return static_cast<double>(v); }
                double operator()(const QString& v) const  { return v.toDouble(); }
            };

            return std::visit(Visitor(), _terminalValue);
        }

        bool isSameTypeAs(const TerminalValueWrapper& other) const
        {
            return _terminalValue.index() == other._terminalValue.index();
        }

        ValueType type() const
        {
            struct Visitor
            {
                ValueType operator()(double) const          { return ValueType::Float; }
                ValueType operator()(int) const             { return ValueType::Int; }
                ValueType operator()(const QString&) const  { return ValueType::String; }
            };

            return std::visit(Visitor(), _terminalValue);
        }

        GraphTransformConfig::TerminalValue operator*() const
        {
            return _terminalValue;
        }
    };

    template<typename E>
    struct AttributesOpVistor : public boost::static_visitor<ElementConditionFn<E>>
    {
        Attribute _lhs;
        Attribute _rhs;

        AttributesOpVistor(const Attribute& lhs, const Attribute& rhs) :
            _lhs(lhs), _rhs(rhs)
        {}

        ElementConditionFn<E> operator()(ConditionFnOp::Equality op) const
        {
            auto comparisonFn = [op](const Attribute& lhs, auto valueOfFn, const Attribute& rhs) -> ElementConditionFn<E>
            {
                switch(op)
                {
                case ConditionFnOp::Equality::Equal:
                    return [lhs, valueOfFn, rhs](E elementId) { return (lhs.*valueOfFn)(elementId) == (rhs.*valueOfFn)(elementId); };
                case ConditionFnOp::Equality::NotEqual:
                    return [lhs, valueOfFn, rhs](E elementId) { return (lhs.*valueOfFn)(elementId) != (rhs.*valueOfFn)(elementId); };
                default:
                    qFatal("Unhandled ConditionFnOp::Equality");
                    return nullptr;
                }
            };

            if(_lhs.valueType() == _rhs.valueType())
            {
                switch(_lhs.valueType())
                {
                case ValueType::Float:  return comparisonFn(_lhs, &Attribute::valueOf<double, E>,  _rhs);
                case ValueType::Int:    return comparisonFn(_lhs, &Attribute::valueOf<int, E>,     _rhs);
                case ValueType::String: return comparisonFn(_lhs, &Attribute::valueOf<QString, E>, _rhs);
                default: return nullptr;
                }
            }

            return comparisonFn(_lhs, &Attribute::stringValueOf<E>, _rhs);
        }

        ElementConditionFn<E> operator()(ConditionFnOp::Numerical op) const
        {
            if(_lhs.valueType() == ValueType::String || _rhs.valueType() == ValueType::String)
                return nullptr; // Can't compare a string attribute numerically

            auto comparisonFn = [op](const Attribute& lhs, auto lhsValueOfFn,
                                     const Attribute& rhs, auto rhsValueOfFn) -> ElementConditionFn<E>
            {
                switch(op)
                {
                case ConditionFnOp::Numerical::LessThan:
                    return [lhs, lhsValueOfFn, rhs, rhsValueOfFn](E elementId) { return (lhs.*lhsValueOfFn)(elementId) < (rhs.*rhsValueOfFn)(elementId); };
                case ConditionFnOp::Numerical::GreaterThan:
                    return [lhs, lhsValueOfFn, rhs, rhsValueOfFn](E elementId) { return (lhs.*lhsValueOfFn)(elementId) > (rhs.*rhsValueOfFn)(elementId); };
                case ConditionFnOp::Numerical::LessThanOrEqual:
                    return [lhs, lhsValueOfFn, rhs, rhsValueOfFn](E elementId) { return (lhs.*lhsValueOfFn)(elementId) <= (rhs.*rhsValueOfFn)(elementId); };
                case ConditionFnOp::Numerical::GreaterThanOrEqual:
                    return [lhs, lhsValueOfFn, rhs, rhsValueOfFn](E elementId) { return (lhs.*lhsValueOfFn)(elementId) >= (rhs.*rhsValueOfFn)(elementId); };
                default:
                    qFatal("Unhandled ConditionFnOp::Numerical");
                    return nullptr;
                }
            };

            if(_lhs.valueType() == ValueType::Float && _rhs.valueType() == ValueType::Float)
                return comparisonFn(_lhs, &Attribute::valueOf<double, E>, _rhs, &Attribute::valueOf<double, E>);

            if(_lhs.valueType() == ValueType::Float && _rhs.valueType() == ValueType::Int)
                return comparisonFn(_lhs, &Attribute::valueOf<double, E>, _rhs, &Attribute::valueOf<int, E>);

            if(_lhs.valueType() == ValueType::Int && _rhs.valueType() == ValueType::Float)
                return comparisonFn(_lhs, &Attribute::valueOf<int, E>, _rhs, &Attribute::valueOf<double, E>);

            if(_lhs.valueType() == ValueType::Int && _rhs.valueType() == ValueType::Int)
                return comparisonFn(_lhs, &Attribute::valueOf<int, E>, _rhs, &Attribute::valueOf<int, E>);

            qFatal("Shouldn't get here");
            return nullptr;
        }

        ElementConditionFn<E> operator()(ConditionFnOp::String op) const
        {
            auto lhs = _lhs;
            auto rhs = _lhs;

            Attribute::ValueOfFn<QString, E> valueOfFn = &Attribute::valueOf<QString, E>;

            if(lhs.valueType() != ValueType::String || rhs.valueType() != ValueType::String)
                valueOfFn = &Attribute::stringValueOf<E>;

            switch(op)
            {
            case ConditionFnOp::String::Includes:
                return [lhs, valueOfFn, rhs](E elementId) { return (lhs.*valueOfFn)(elementId).contains((rhs.*valueOfFn)(elementId)); };
            case ConditionFnOp::String::Excludes:
                return [lhs, valueOfFn, rhs](E elementId) { return !(lhs.*valueOfFn)(elementId).contains((rhs.*valueOfFn)(elementId)); };
            case ConditionFnOp::String::Starts:
                return [lhs, valueOfFn, rhs](E elementId) { return (lhs.*valueOfFn)(elementId).startsWith((rhs.*valueOfFn)(elementId)); };
            case ConditionFnOp::String::Ends:
                return [lhs, valueOfFn, rhs](E elementId) { return (lhs.*valueOfFn)(elementId).endsWith((rhs.*valueOfFn)(elementId)); };
            case ConditionFnOp::String::MatchesRegex:
            case ConditionFnOp::String::MatchesRegexCaseInsensitive:
            {
                auto reOption = op == ConditionFnOp::String::MatchesRegexCaseInsensitive ?
                            QRegularExpression::CaseInsensitiveOption :
                            QRegularExpression::NoPatternOption;

                return [lhs, valueOfFn, rhs, reOption](E elementId)
                {
                    QRegularExpression re((rhs.*valueOfFn)(elementId), reOption);
                    if(!re.isValid())
                        return false; // Regex isn't valid

                    return re.match((lhs.*valueOfFn)(elementId)).hasMatch();
                };
            }
            default:
                qFatal("Unhandled ConditionFnOp::String");
                return nullptr;
            }
        }
    };

    template<typename E>
    struct AttributeValueOpVistor : public boost::static_visitor<ElementConditionFn<E>>
    {
        Attribute _lhs;
        TerminalValueWrapper _rhs;
        bool _operandsAreSwitched;

        AttributeValueOpVistor(const Attribute& lhs, TerminalValueWrapper rhs,
                               bool operandsAreSwitched = false) :
            _lhs(lhs), _rhs(std::move(rhs)), _operandsAreSwitched(operandsAreSwitched)
        {}

        ElementConditionFn<E> operator()(ConditionFnOp::Equality op) const
        {
            auto comparisonFn = [op](const Attribute& attribute, auto valueOfFn, auto value) -> ElementConditionFn<E>
            {
                switch(op)
                {
                case ConditionFnOp::Equality::Equal:
                    return [attribute, valueOfFn, value](E elementId) { return (attribute.*valueOfFn)(elementId) == value; };
                case ConditionFnOp::Equality::NotEqual:
                    return [attribute, valueOfFn, value](E elementId) { return (attribute.*valueOfFn)(elementId) != value; };
                default:
                    qFatal("Unhandled ConditionFnOp::Equality");
                    return nullptr;
                }
            };

            if(_lhs.valueType() == _rhs.type())
            {
                switch(_lhs.valueType())
                {
                case ValueType::Float:  return comparisonFn(_lhs, &Attribute::valueOf<double, E>,  std::get<double>(*_rhs));
                case ValueType::Int:    return comparisonFn(_lhs, &Attribute::valueOf<int, E>,     std::get<int>(*_rhs));
                case ValueType::String: return comparisonFn(_lhs, &Attribute::valueOf<QString, E>, std::get<QString>(*_rhs));
                default: return nullptr;
                }
            }

            return comparisonFn(_lhs, &Attribute::stringValueOf<E>, _rhs.toString());
        }

        ElementConditionFn<E> operator()(ConditionFnOp::Numerical op) const
        {
            if(_lhs.valueType() == ValueType::String)
                return nullptr; // Can't compare a string attribute with a number

            if(_operandsAreSwitched)
            {
                switch(op)
                {
                case ConditionFnOp::Numerical::LessThan:            op = ConditionFnOp::Numerical::GreaterThanOrEqual; break;
                case ConditionFnOp::Numerical::GreaterThan:         op = ConditionFnOp::Numerical::LessThanOrEqual; break;
                case ConditionFnOp::Numerical::LessThanOrEqual:     op = ConditionFnOp::Numerical::GreaterThan; break;
                case ConditionFnOp::Numerical::GreaterThanOrEqual:  op = ConditionFnOp::Numerical::LessThan; break;
                }
            }

            auto comparisonFn = [op](const Attribute& attribute, auto value) -> ElementConditionFn<E>
            {
                switch(op)
                {
                case ConditionFnOp::Numerical::LessThan:
                    return [attribute, value](E elementId) { return attribute.template valueOf<decltype(value), E>(elementId) < value; };
                case ConditionFnOp::Numerical::GreaterThan:
                    return [attribute, value](E elementId) { return attribute.template valueOf<decltype(value), E>(elementId) > value; };
                case ConditionFnOp::Numerical::LessThanOrEqual:
                    return [attribute, value](E elementId) { return attribute.template valueOf<decltype(value), E>(elementId) <= value; };
                case ConditionFnOp::Numerical::GreaterThanOrEqual:
                    return [attribute, value](E elementId) { return attribute.template valueOf<decltype(value), E>(elementId) >= value; };
                default:
                    qFatal("Unhandled ConditionFnOp::Numerical");
                    return nullptr;
                }
            };

            if(_lhs.valueType() != _rhs.type())
            {
                auto numberValue = _rhs.toDouble();

                switch(_lhs.valueType())
                {
                case ValueType::Float:  return comparisonFn(_lhs, numberValue);
                case ValueType::Int:    return comparisonFn(_lhs, static_cast<int>(numberValue));
                default: return nullptr;
                }
            }
            else
            {
                switch(_lhs.valueType())
                {
                case ValueType::Float:  return comparisonFn(_lhs, std::get<double>(*_rhs));
                case ValueType::Int:    return comparisonFn(_lhs, std::get<int>(*_rhs));
                default: return nullptr;
                }
            }
        }

        ElementConditionFn<E> operator()(ConditionFnOp::String op) const
        {
            auto attribute = _lhs;

            Attribute::ValueOfFn<QString, E> valueOfFn = &Attribute::valueOf<QString, E>;

            if(attribute.valueType() != ValueType::String)
                valueOfFn = &Attribute::stringValueOf<E>;

            auto value = _rhs.toString();

            switch(op)
            {
            case ConditionFnOp::String::Includes:
                return [attribute, valueOfFn, value](E elementId) { return (attribute.*valueOfFn)(elementId).contains(value); };
            case ConditionFnOp::String::Excludes:
                return [attribute, valueOfFn, value](E elementId) { return !(attribute.*valueOfFn)(elementId).contains(value); };
            case ConditionFnOp::String::Starts:
                return [attribute, valueOfFn, value](E elementId) { return (attribute.*valueOfFn)(elementId).startsWith(value); };
            case ConditionFnOp::String::Ends:
                return [attribute, valueOfFn, value](E elementId) { return (attribute.*valueOfFn)(elementId).endsWith(value); };
            case ConditionFnOp::String::MatchesRegex:
            case ConditionFnOp::String::MatchesRegexCaseInsensitive:
            {
                auto reOption = op == ConditionFnOp::String::MatchesRegexCaseInsensitive ?
                            QRegularExpression::CaseInsensitiveOption :
                            QRegularExpression::NoPatternOption;

                QRegularExpression re(value, reOption);
                if(!re.isValid())
                    return nullptr; // Regex isn't valid

                return [attribute, valueOfFn, re](E elementId) { return re.match((attribute.*valueOfFn)(elementId)).hasMatch(); };
            }
            default:
                qFatal("Unhandled ConditionFnOp::String");
                return nullptr;
            }
        }
    };

    template<typename E>
    struct ValuesOpVistor
    {
        TerminalValueWrapper _lhs;
        TerminalValueWrapper _rhs;

        ValuesOpVistor(TerminalValueWrapper lhs, TerminalValueWrapper rhs) :
            _lhs(std::move(lhs)), _rhs(std::move(rhs))
        {}

        ElementConditionFn<E> f(bool condition) const
        {
            return [condition](E) { return condition; };
        }

        ElementConditionFn<E> operator()(ConditionFnOp::Equality op) const
        {
            auto comparisonFn = [this, op](const auto& lhs, const auto& rhs) -> ElementConditionFn<E>
            {
                switch(op)
                {
                case ConditionFnOp::Equality::Equal:    return this->f(lhs == rhs);
                case ConditionFnOp::Equality::NotEqual: return this->f(lhs != rhs);
                default:
                    qFatal("Unhandled ConditionFnOp::Equality");
                    return nullptr;
                }
            };

            if(_lhs.isSameTypeAs(_rhs))
                return comparisonFn(*_lhs, *_rhs);

            return comparisonFn(_lhs.toString(), _rhs.toString());
        }

        ElementConditionFn<E> operator()(ConditionFnOp::Numerical op) const
        {
            auto comparisonFn = [this, op](const auto& lhs, const auto& rhs) -> ElementConditionFn<E>
            {
                switch(op)
                {
                case ConditionFnOp::Numerical::LessThan:            return this->f(lhs < rhs);
                case ConditionFnOp::Numerical::GreaterThan:         return this->f(lhs > rhs);
                case ConditionFnOp::Numerical::LessThanOrEqual:     return this->f(lhs <= rhs);
                case ConditionFnOp::Numerical::GreaterThanOrEqual:  return this->f(lhs >= rhs);
                default:
                    qFatal("Unhandled ConditionFnOp::Numerical");
                    return nullptr;
                }
            };

            if(_lhs.isSameTypeAs(_rhs) && _lhs.type() != ValueType::String)
                return comparisonFn(*_lhs, *_rhs);

            return comparisonFn(_lhs.toString(), _rhs.toString());
        }

        ElementConditionFn<E> operator()(ConditionFnOp::String op) const
        {
            QString lhs = _lhs.toString();
            QString rhs = _rhs.toString();

            switch(op)
            {
            case ConditionFnOp::String::Includes:       return f(lhs.contains(rhs));
            case ConditionFnOp::String::Excludes:       return f(!lhs.contains(rhs));
            case ConditionFnOp::String::Starts:         return f(lhs.startsWith(rhs));
            case ConditionFnOp::String::Ends:           return f(lhs.endsWith(rhs));
            case ConditionFnOp::String::MatchesRegex:
            case ConditionFnOp::String::MatchesRegexCaseInsensitive:
            {
                auto reOption = op == ConditionFnOp::String::MatchesRegexCaseInsensitive ?
                            QRegularExpression::CaseInsensitiveOption :
                            QRegularExpression::NoPatternOption;

                QRegularExpression re(rhs, reOption);
                if(!re.isValid())
                    return nullptr; // Regex isn't valid

                return f(re.match(lhs).hasMatch());
            }
            default:
                qFatal("Unhandled ConditionFnOp::String");
                return nullptr;
            }
        }
    };

    template<typename E>
    struct ConditionVisitor : public boost::static_visitor<ElementConditionFn<E>>
    {
        ElementType _elementType;
        const GraphModel* _graphModel;
        bool _strictTyping = false;

        ConditionVisitor(ElementType elementType, const GraphModel& graphModel, bool strictTyping = false) :
            _elementType(elementType),
            _graphModel(&graphModel),
            _strictTyping(strictTyping)
        {}

        ElementConditionFn<E> operator()(GraphTransformConfig::NoCondition) const
        {
            // Not a condition
            return nullptr;
        }

        using ResolvedTerminalValue = std::variant<double, int, QString, Attribute>;

        ResolvedTerminalValue resolvedTerminalValue(const GraphTransformConfig::TerminalValue& terminalValue) const
        {
            struct Visitor
            {
                const GraphModel* _graphModel;

                explicit Visitor(const GraphModel* graphModel) :
                    _graphModel(graphModel)
                {}

                ResolvedTerminalValue operator()(double v) const { return v; }
                ResolvedTerminalValue operator()(int v) const { return v; }
                ResolvedTerminalValue operator()(const QString& v) const
                {
                    if(GraphTransformConfigParser::isAttributeName(v))
                        return _graphModel->attributeValueByName(v);

                    return v;
                }
            };

            return std::visit(Visitor(_graphModel), terminalValue);
        }

        Attribute attributeFromValue(const ResolvedTerminalValue& resolvedTerminalValue) const
        {
            const auto* attribute = std::get_if<Attribute>(&resolvedTerminalValue);

            if(attribute != nullptr)
                return *attribute;

            return {}; // Not an attribute
        }

        ValueType typeOf(const ResolvedTerminalValue& resolvedTerminalValue) const
        {
            struct Visitor
            {
                const GraphModel* _graphModel;

                explicit Visitor(const GraphModel* graphModel) :
                    _graphModel(graphModel)
                {}

                ValueType operator()(double) const          { return ValueType::Float; }
                ValueType operator()(int) const             { return ValueType::Int; }
                ValueType operator()(const QString&) const  { return ValueType::String; }

                ValueType operator()(const Attribute& attribute) const
                {
                    return attribute.valueType();
                }
            };

            return std::visit(Visitor(_graphModel), resolvedTerminalValue);
        }

        bool isUnknownAttribute(const ResolvedTerminalValue& resolvedTerminalValue) const
        {
            const Attribute* attribute = std::get_if<Attribute>(&resolvedTerminalValue);

            return attribute != nullptr && !attribute->isValid();
        }

        ElementConditionFn<E> operator()(const GraphTransformConfig::TerminalCondition& terminalCondition) const
        {
            const auto lhs = resolvedTerminalValue(terminalCondition._lhs);
            const auto rhs = resolvedTerminalValue(terminalCondition._rhs);

            if(isUnknownAttribute(lhs) || isUnknownAttribute(rhs))
                return nullptr; // Unknown attribute

            if(_strictTyping)
            {
                auto lhsType = typeOf(lhs);
                auto rhsType = typeOf(rhs);

                if(lhsType != rhsType)
                    return nullptr; // Types do not match

                auto validOps = GraphTransformConfigParser::ops(lhsType);
                bool opIsValid = std::any_of(validOps.begin(), validOps.end(),
                [&terminalCondition](const auto& validOp)
                {
                    return terminalCondition._op == GraphTransformConfigParser::stringToOp(validOp);
                });

                if(!opIsValid)
                    return nullptr; // Operator doesn't apply to the type of the operands
            }

            auto lhsAttribute = attributeFromValue(lhs);
            auto rhsAttribute = attributeFromValue(rhs);

            if(lhsAttribute.isValid() && !lhsAttribute.template isOfElementType<E>())
                return nullptr; // Type doesn't match what we're creating for

            if(rhsAttribute.isValid() && !rhsAttribute.template isOfElementType<E>())
                return nullptr; // Type doesn't match what we're creating for

            if(lhsAttribute.isValid() && rhsAttribute.isValid())
            {
                // Both sides are attributes
                AttributesOpVistor<E> visitor(lhsAttribute, rhsAttribute);
                return std::visit(visitor, terminalCondition._op);
            }

            if(!lhsAttribute.isValid() && !rhsAttribute.isValid())
            {
                // Neither side is an attribute
                ValuesOpVistor<E> visitor(terminalCondition._lhs, terminalCondition._rhs);
                return std::visit(visitor, terminalCondition._op);
            }

            if(lhsAttribute.isValid())
            {
                // Left hand side is an attribute
                AttributeValueOpVistor<E> visitor(lhsAttribute, terminalCondition._rhs, false);
                return std::visit(visitor, terminalCondition._op);
            }

            if(rhsAttribute.isValid())
            {
                // Right hand side is an attribute
                AttributeValueOpVistor<E> visitor(rhsAttribute, terminalCondition._lhs, true);
                return std::visit(visitor, terminalCondition._op);
            }

            return nullptr;

        }

        ElementConditionFn<E> operator()(const GraphTransformConfig::UnaryCondition& unaryCondition) const
        {
            const auto lhs = resolvedTerminalValue(unaryCondition._lhs);

            if(isUnknownAttribute(lhs))
                return nullptr; // Unknown attribute

            auto attribute = attributeFromValue(lhs);

            if(!attribute.isValid())
                return nullptr; // Not an attribute

            if(!attribute.template isOfElementType<E>())
                return nullptr; // Type doesn't match what we're creating for

            switch(unaryCondition._op)
            {
            case ConditionFnOp::Unary::HasValue:
                return [attribute](E elementId) { return !attribute.template valueMissingOf<E>(elementId); };
            default:
                qFatal("Unhandled ConditionFnOp::Unary");
                return nullptr;
            }

            return nullptr;
        }

        ElementConditionFn<E> operator()(const GraphTransformConfig::CompoundCondition& compoundCondition) const
        {
            auto lhs = boost::apply_visitor(ConditionVisitor<E>(_elementType, *_graphModel), compoundCondition._lhs);
            auto rhs = boost::apply_visitor(ConditionVisitor<E>(_elementType, *_graphModel), compoundCondition._rhs);

            if(lhs == nullptr || rhs == nullptr)
                return nullptr;

            switch(compoundCondition._op)
            {
            case ConditionFnOp::Logical::And:
                return [lhs, rhs](E elementId) { return lhs(elementId) && rhs(elementId); };
            case ConditionFnOp::Logical::Or:
                return [lhs, rhs](E elementId) { return lhs(elementId) || rhs(elementId); };
            default:
                qFatal("Unhandled BinaryOp");
                return nullptr;
            }

            return nullptr;
        }
    };

public:
    static auto node(const GraphModel& graphModel,
                     const GraphTransformConfig::Condition& condition)
    {
        return boost::apply_visitor(ConditionVisitor<NodeId>(ElementType::Node, graphModel), condition);
    }

    static auto edge(const GraphModel& graphModel,
                     const GraphTransformConfig::Condition& condition)
    {
        return boost::apply_visitor(ConditionVisitor<EdgeId>(ElementType::Edge, graphModel), condition);
    }

    static auto component(const GraphModel& graphModel,
                          const GraphTransformConfig::Condition& condition)
    {
        return boost::apply_visitor(ConditionVisitor<const IGraphComponent&>(ElementType::Component, graphModel), condition);
    }

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

    template<typename Op, typename Value>
    static auto node(const Attribute& attribute, Op op, Value value)
    {
        GraphTransformConfig::TerminalOp terminalOp = op;
        AttributeValueOpVistor<NodeId> visitor(attribute, TerminalValueWrapper(value), false);
        return std::visit(visitor, terminalOp);
    }

    template<typename Op, typename Value>
    static auto edge(const Attribute& attribute, Op op, Value value)
    {
        GraphTransformConfig::TerminalOp terminalOp = op;
        AttributeValueOpVistor<EdgeId> visitor(attribute, TerminalValueWrapper(value), false);
        return std::visit(visitor, terminalOp);
    }

    template<typename Op, typename Value>
    static auto component(const Attribute& attribute, Op op, Value value)
    {
        GraphTransformConfig::TerminalOp terminalOp = op;
        AttributeValueOpVistor<const IGraphComponent&> visitor(attribute, TerminalValueWrapper(value), false);
        return std::visit(visitor, terminalOp);
    }

    template<typename E, typename Op, typename Value>
    static auto elementType(const Attribute& attribute, Op op, Value value)
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
