#ifndef GRAPHTRANSFORMCONFIG_H
#define GRAPHTRANSFORMCONFIG_H

#include <QString>
#include <QVariantMap>

#include <vector>

#include "thirdparty/boost/boost_disable_warnings.h"
#include "boost/variant.hpp"
#include "boost/variant/recursive_wrapper.hpp"
#include "thirdparty/boost/boost_enable_warnings.h"

struct GraphTransformConfig
{
    enum class BinaryOp { Or, And };
    enum class NumericalOp
    {
        Equal,
        NotEqual,
        LessThan,
        GreaterThan,
        LessThanOrEqual,
        GreaterThanOrEqual
    };
    enum class StringOp
    {
        Equal,
        NotEqual,
        Includes,
        Excludes,
        Starts,
        Ends,
        MatchesRegex
    };

    struct FloatOpValue
    {
        NumericalOp _op;
        double _value;

        bool operator==(const FloatOpValue& other) const;
    };

    struct IntOpValue
    {
        NumericalOp _op;
        int _value;

        bool operator==(const IntOpValue& other) const;
    };

    struct StringOpValue
    {
        StringOp _op;
        QString _value;

        bool operator==(const StringOpValue& other) const;
    };

    using OpValue = boost::variant<FloatOpValue, IntOpValue, StringOpValue>;

    struct TerminalCondition
    {
        QString _field;
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
        BinaryOp _op;
        Condition _rhs;

        bool operator==(const CompoundCondition& other) const;
        QString opAsString() const;
    };

    using ParameterValue = boost::variant<double, QString>;
    struct Parameter
    {
        QString _name;
        ParameterValue _value;

        bool operator==(const Parameter& other) const;
        QString valueAsString() const;
    };

    std::vector<QString> _metaAttributes;
    QString _action;
    std::vector<Parameter> _parameters;
    Condition _condition;

    QVariantMap conditionAsVariantMap() const;
    QVariantMap asVariantMap() const;

    bool operator==(const GraphTransformConfig& other) const;
    bool operator!=(const GraphTransformConfig& other) const;
    bool isMetaAttributeSet(const QString& metaAttribute) const;
};

#endif // GRAPHTRANSFORMCONFIG_H
