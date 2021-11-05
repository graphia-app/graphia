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

#ifndef IATTRIBUTE_H
#define IATTRIBUTE_H

#include "iattributerange.h"
#include "valuetype.h"

#include "shared/graph/elementid.h"
#include "shared/graph/elementtype.h"
#include "shared/utils/qmlenum.h"

#include <functional>
#include <vector>
#include <variant>

#include <QString>
#include <QVariant>

DEFINE_QML_ENUM(
    Q_GADGET, AttributeFlag,
    None                    = 0x0,

    // Automatically set the range
    AutoRange               = 0x1,

    // Visualise on a per-component basis, by default
    VisualiseByComponent    = 0x2,

    // Indicates this is a dynamically created attribute; set automatically
    Dynamic                 = 0x4,

    // Track the set of shared values held by the attribute
    FindShared              = 0x8,

    // Can't be used during transform
    DisableDuringTransform  = 0x10,

    // Can be searched by the various find methods
    Searchable              = 0x20);

class IGraphComponent;

class IAttribute
{
public:
    virtual ~IAttribute() = default;

    template<typename T, typename E>
    using ValueFn = std::variant<
        void*,
        std::function<T(E)>,
        std::function<T(E, const IAttribute&)>
    >;

    template<typename E>
    using SetValueFn = std::function<void(E, const QString&)>;

    virtual int intValueOf(NodeId nodeId) const = 0;
    virtual int intValueOf(EdgeId edgeId) const = 0;
    virtual int intValueOf(const IGraphComponent& graphComponent) const = 0;

    virtual double floatValueOf(NodeId nodeId) const = 0;
    virtual double floatValueOf(EdgeId edgeId) const = 0;
    virtual double floatValueOf(const IGraphComponent& graphComponent) const = 0;

    virtual double numericValueOf(NodeId nodeId) const = 0;
    virtual double numericValueOf(EdgeId edgeId) const = 0;
    virtual double numericValueOf(const IGraphComponent& graphComponent) const = 0;

    virtual QString stringValueOf(NodeId nodeId) const = 0;
    virtual QString stringValueOf(EdgeId edgeId) const = 0;
    virtual QString stringValueOf(const IGraphComponent& graphComponent) const = 0;

    template<typename E>
    QVariant valueOf(E elementId) const
    {
        switch(valueType())
        {
        case ValueType::Int:    return intValueOf(elementId);
        case ValueType::Float:  return floatValueOf(elementId);
        case ValueType::String: return stringValueOf(elementId);
        default:                return {};
        }
    }

    virtual bool valueMissingOf(NodeId nodeId) const = 0;
    virtual bool valueMissingOf(EdgeId edgeId) const = 0;
    virtual bool valueMissingOf(const IGraphComponent& graphComponent) const = 0;

    virtual void setValueOf(NodeId nodeId, const QString& value) const = 0;
    virtual void setValueOf(EdgeId edgeId, const QString& value) const = 0;
    virtual void setValueOf(const IGraphComponent& graphComponent, const QString& value) const = 0;

    virtual IAttribute& setIntValueFn(ValueFn<int, NodeId> valueFn) = 0;
    virtual IAttribute& setIntValueFn(ValueFn<int, EdgeId> valueFn) = 0;
    virtual IAttribute& setIntValueFn(ValueFn<int, const IGraphComponent&> valueFn) = 0;

    virtual IAttribute& setFloatValueFn(ValueFn<double, NodeId> valueFn) = 0;
    virtual IAttribute& setFloatValueFn(ValueFn<double, EdgeId> valueFn) = 0;
    virtual IAttribute& setFloatValueFn(ValueFn<double, const IGraphComponent&> valueFn) = 0;

    virtual IAttribute& setStringValueFn(ValueFn<QString, NodeId> valueFn) = 0;
    virtual IAttribute& setStringValueFn(ValueFn<QString, EdgeId> valueFn) = 0;
    virtual IAttribute& setStringValueFn(ValueFn<QString, const IGraphComponent&> valueFn) = 0;

    virtual IAttribute& setValueMissingFn(ValueFn<bool, NodeId> missingFn) = 0;
    virtual IAttribute& setValueMissingFn(ValueFn<bool, EdgeId> missingFn) = 0;
    virtual IAttribute& setValueMissingFn(ValueFn<bool, const IGraphComponent&> missingFn) = 0;

    virtual IAttribute& setSetValueFn(SetValueFn<NodeId> setValueFn) = 0;
    virtual IAttribute& setSetValueFn(SetValueFn<EdgeId> setValueFn) = 0;
    virtual IAttribute& setSetValueFn(SetValueFn<const IGraphComponent&> setValueFn) = 0;

    virtual ValueType valueType() const = 0;
    virtual ElementType elementType() const = 0;

    virtual IAttributeRange<int>& intRange() = 0;
    virtual IAttributeRange<double>& floatRange() = 0;
    virtual const IAttributeRange<double>& numericRange() const = 0;

    virtual bool testFlag(AttributeFlag flag) const = 0;
    virtual IAttribute& setFlag(AttributeFlag flag) = 0;
    virtual IAttribute& resetFlag(AttributeFlag flag) = 0;

    struct SharedValue
    {
        QString _value;
        int _count;
    };

    virtual std::vector<SharedValue> sharedValues() const = 0;

    virtual bool userDefined() const = 0;
    virtual IAttribute& setUserDefined(bool userDefined) = 0;

    virtual bool editable() const = 0;

    virtual QString description() const = 0;
    virtual IAttribute& setDescription(const QString& description) = 0;

    virtual bool isValid() const = 0;

    virtual QString parameterValue() const = 0;
    virtual bool setParameterValue(const QString& value) = 0;

    virtual bool hasParameter() const = 0;
    virtual QStringList validParameterValues() const = 0;
    virtual IAttribute& setValidParameterValues(const QStringList& values) = 0;
};

#endif // IATTRIBUTE_H
