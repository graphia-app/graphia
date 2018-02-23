#ifndef IATTRIBUTE_H
#define IATTRIBUTE_H

#include "iattributerange.h"
#include "valuetype.h"

#include "shared/graph/elementid.h"
#include "shared/graph/elementtype.h"

#include <functional>
#include <vector>

#include <QString>
#include <QVariant>

enum class AttributeFlag
{
    None                    = 0x0,

    // Automatically set the range
    AutoRange               = 0x1,

    // With multielements, don't process the tails in transforms
    IgnoreTails             = 0x2,

    // Indicates this is a dynamically created attribute; set automatically
    Dynamic                 = 0x4,

    // Track the set of unique values held by the attribute
    FindUnique              = 0x8,
};

class IGraphComponent;

class IAttribute
{
public:
    virtual ~IAttribute() = default;

    template<typename T, typename E> using ValueFn = std::function<T(E)>;

    virtual int intValueOf(NodeId nodeId) const = 0;
    virtual int intValueOf(EdgeId edgeId) const = 0;
    virtual int intValueOf(const IGraphComponent& graphComponent) const = 0;

    virtual double floatValueOf(NodeId nodeId) const = 0;
    virtual double floatValueOf(EdgeId edgeId) const = 0;
    virtual double floatValueOf(const IGraphComponent& graphComponent) const = 0;

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

    virtual ValueType valueType() const = 0;
    virtual ElementType elementType() const = 0;

    virtual IAttributeRange<int>& intRange() = 0;
    virtual IAttributeRange<double>& floatRange() = 0;
    virtual const IAttributeRange<double>& numericRange() const = 0;

    virtual bool testFlag(AttributeFlag flag) const = 0;
    virtual IAttribute& setFlag(AttributeFlag flag) = 0;
    virtual IAttribute& resetFlag(AttributeFlag flag) = 0;

    struct UniqueValue
    {
        QString _value;
        int _count;
    };

    virtual std::vector<UniqueValue> uniqueValues() const = 0;

    virtual bool searchable() const = 0;
    virtual IAttribute& setSearchable(bool searchable) = 0;

    virtual bool userDefined() const = 0;
    virtual IAttribute& setUserDefined(bool userDefined) = 0;

    virtual QString description() const = 0;
    virtual IAttribute& setDescription(const QString& description) = 0;
};

#endif // IATTRIBUTE_H
