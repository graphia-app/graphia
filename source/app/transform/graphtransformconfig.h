#ifndef GRAPHTRANSFORMCONFIG_H
#define GRAPHTRANSFORMCONFIG_H

#include "attributes/condtionfnops.h"

#include <QString>
#include <QVariantMap>

#include <vector>

#include "thirdparty/boost/boost_disable_warnings.h"
#include "boost/variant.hpp"
#include "boost/variant/recursive_wrapper.hpp"
#include "thirdparty/boost/boost_enable_warnings.h"

struct GraphTransformConfig
{
    struct FloatOpValue
    {
        ConditionFnOp::Numerical _op;
        double _value;

        bool operator==(const FloatOpValue& other) const;
    };

    struct IntOpValue
    {
        ConditionFnOp::Numerical _op;
        int _value;

        bool operator==(const IntOpValue& other) const;
    };

    struct StringOpValue
    {
        ConditionFnOp::String _op;
        QString _value;

        bool operator==(const StringOpValue& other) const;
    };

    using UnaryOp = ConditionFnOp::Unary;

    using OpValue = boost::variant<FloatOpValue, IntOpValue, StringOpValue, UnaryOp>;

    struct TerminalCondition
    {
        QString _attributeName;
        OpValue _opValue;

        bool operator==(const TerminalCondition& other) const;
        QString opAsString() const;
        QString valueAsString() const;
    };

    struct CompoundCondition;
    using Condition = boost::variant<
        TerminalCondition,
        boost::recursive_wrapper<CompoundCondition>>;

    struct CompoundCondition
    {
        Condition _lhs;
        ConditionFnOp::Binary _op;
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

    bool hasCondition() const;
    QVariantMap conditionAsVariantMap() const;
    QVariantMap asVariantMap() const;

    std::vector<QString> attributeNames() const;

    bool operator==(const GraphTransformConfig& other) const;
    bool operator!=(const GraphTransformConfig& other) const;
    bool isFlagSet(const QString& flag) const;
};

#endif // GRAPHTRANSFORMCONFIG_H
