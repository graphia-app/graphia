/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

#ifndef GRAPHTRANSFORMCONFIG_H
#define GRAPHTRANSFORMCONFIG_H

#include "app/attributes/condtionfnops.h"

#include "shared/attributes/valuetype.h"

#include "shared/utils/namedbool.h"
#include "shared/utils/recursivevalue.h"

#include <QString>
#include <QVariantMap>

#include <compare>
#include <functional>
#include <vector>
#include <variant>

class GraphTransformFactory;

struct GraphTransformConfig
{
    using AttributeDisplayNameFn = std::function<QString(const QString&)>;
    using AttributeDisplayValueFn = std::function<QString(const QString& attributeName, const QString& value)>;

    // An attribute named in a condition, disambiguated from a plain string value
    struct AttributeRef
    {
        QString _name;

        bool operator==(const AttributeRef& other) const = default;
        std::weak_ordering operator<=>(const AttributeRef& other) const = default;
    };

    using TerminalValue = std::variant<double, int, QString, AttributeRef>;
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
    using Condition = std::variant<
        NoCondition,
        TerminalCondition,
        UnaryCondition,
        RecursiveValue<CompoundCondition>>;

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
        QString valueAsString(NamedBool<"asWritten"> asWritten = "asWritten"_no) const;
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
    QString conditionAsString(NamedBool<"forDisplay"> forDisplay = "forDisplay"_no,
        const AttributeDisplayNameFn& attributeDisplayName = {},
        const AttributeDisplayValueFn& attributeDisplayValue = {}) const;

    QVariantMap asVariantMap(const GraphTransformFactory* factory) const;
    // When forDisplay, attributeDisplayName gives the name to show for each attribute, and
    // attributeDisplayValue the value to show where one is compared with an attribute, if supplied
    QString asString(NamedBool<"forDisplay"> forDisplay = "forDisplay"_no,
        const GraphTransformFactory* factory = nullptr,
        const AttributeDisplayNameFn& attributeDisplayName = {},
        const AttributeDisplayValueFn& attributeDisplayValue = {}) const;

    std::vector<QString> referencedAttributeNames() const;

    bool equals(const GraphTransformConfig& other,
        NamedBool<"ignoreInertFlags"> ignoreInertFlags = "ignoreInertFlags"_yes) const;
    bool isFlagSet(const QString& flag) const;

private:
    QString asDisplayString(const GraphTransformFactory* factory, const AttributeDisplayNameFn& attributeDisplayName,
        const AttributeDisplayValueFn& attributeDisplayValue) const;
};

#endif // GRAPHTRANSFORMCONFIG_H
