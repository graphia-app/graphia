#ifndef GRAPHTRANSFORMCONFIG_H
#define GRAPHTRANSFORMCONFIG_H

#include "attributes/condtionfnops.h"
#include "shared/attributes/valuetype.h"

#include <QString>
#include <QVariantMap>

#include <vector>

#include "thirdparty/boost/boost_disable_warnings.h"
#include "boost/variant.hpp"
#include "boost/variant/recursive_wrapper.hpp"
#include "thirdparty/boost/boost_enable_warnings.h"

struct GraphTransformConfig
{
    using TerminalValue = boost::variant<double, int, QString>;
    using TerminalOp = boost::variant<ConditionFnOp::Equality, ConditionFnOp::Numerical, ConditionFnOp::String>;

    struct TerminalCondition
    {
        TerminalValue _lhs;
        TerminalOp _op;
        TerminalValue _rhs;

        bool operator==(const TerminalCondition& other) const;
        QString opAsString() const;
    };

    struct UnaryCondition
    {
        TerminalValue _lhs;
        ConditionFnOp::Unary _op;

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
        Condition _lhs;
        ConditionFnOp::Logical _op;
        Condition _rhs;

        bool operator==(const CompoundCondition& other) const;
        QString opAsString() const;
    };

    using ParameterValue = boost::variant<double, int, QString>;
    struct Parameter
    {
        QString _name;
        ParameterValue _value;

        bool operator==(const Parameter& other) const;
        QString valueAsString() const;
    };

    std::vector<QString> _flags;
    QString _action;
    std::vector<Parameter> _parameters;
    Condition _condition;

    const Parameter* parameterByName(const QString& name) const;

    bool hasCondition() const;
    QVariantMap conditionAsVariantMap() const;
    QVariantMap asVariantMap() const;

    std::vector<QString> attributeNames() const;

    bool operator==(const GraphTransformConfig& other) const;
    bool operator!=(const GraphTransformConfig& other) const;
    bool isFlagSet(const QString& flag) const;
};

#endif // GRAPHTRANSFORMCONFIG_H
