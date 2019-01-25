#ifndef GRAPHTRANSFORMCONFIG_H
#define GRAPHTRANSFORMCONFIG_H

#include "attributes/condtionfnops.h"
#include "shared/attributes/valuetype.h"

#include <QString>
#include <QVariantMap>

#include <vector>
#include <variant>

#include <boost/variant.hpp>
#include <boost/variant/recursive_wrapper.hpp>

struct GraphTransformConfig
{
    using TerminalValue = std::variant<double, int, QString>;
    using TerminalOp = std::variant<ConditionFnOp::Equality, ConditionFnOp::Numerical, ConditionFnOp::String>;

    struct TerminalCondition
    {
        TerminalValue _lhs = 0;
        TerminalOp _op = ConditionFnOp::Equality::Equal;
        TerminalValue _rhs = 0;

        bool operator==(const TerminalCondition& other) const;
        QString opAsString() const;
    };

    struct UnaryCondition
    {
        TerminalValue _lhs = 0;
        ConditionFnOp::Unary _op = ConditionFnOp::Unary::HasValue;

        bool operator==(const UnaryCondition& other) const;
        QString opAsString() const;
    };

    struct NoCondition { bool operator==(const NoCondition) const { return true; } };
    struct CompoundCondition;
    using Condition = boost::variant<
        NoCondition,
        TerminalCondition,
        UnaryCondition,
        boost::recursive_wrapper<CompoundCondition>>;

    struct CompoundCondition
    {
        Condition _lhs = NoCondition{};
        ConditionFnOp::Logical _op = ConditionFnOp::Logical::And;
        Condition _rhs = NoCondition{};

        bool operator==(const CompoundCondition& other) const;
        QString opAsString() const;
    };

    using ParameterValue = std::variant<double, int, QString>;
    struct Parameter
    {
        QString _name;
        ParameterValue _value = 0;

        bool operator==(const Parameter& other) const;
        QString valueAsString(bool addQuotes = false) const;
    };

    std::vector<QString> _flags;
    QString _action;
    std::vector<QString> _attributes;
    std::vector<Parameter> _parameters;
    Condition _condition;

    const std::vector<QString>& attributeNames() const;
    bool hasParameter(const QString& name) const;
    const Parameter* parameterByName(const QString& name) const;
    bool parameterHasValue(const QString& name, const QString& value) const;
    void setParameterValue(const QString& name, const ParameterValue& value);

    bool hasCondition() const;

    QVariantMap conditionAsVariantMap() const;
    QString conditionAsString() const;

    QVariantMap asVariantMap() const;
    QString asString() const;

    std::vector<QString> referencedAttributeNames() const;

    bool operator==(const GraphTransformConfig& other) const;
    bool operator!=(const GraphTransformConfig& other) const;
    bool isFlagSet(const QString& flag) const;
};

#endif // GRAPHTRANSFORMCONFIG_H
