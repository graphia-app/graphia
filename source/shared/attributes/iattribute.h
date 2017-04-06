#ifndef IATTRIBUTE_H
#define IATTRIBUTE_H

#include "iattributerange.h"

#include "shared/graph/elementid.h"
#include "shared/graph/igraphcomponent.h"

#include <functional>

enum class AttributeFlag
{
    None                    = 0x0,

    // Automatically set the range...
    AutoRangeMutable        = 0x1, // ...using the MutableGraph ElementIds
    AutoRangeTransformed    = 0x2, // ...using the TransformedGraph ElementIds

    // With multielements, don't process the tails in transforms
    IgnoreTails             = 0x4,

    // Indicates this is a dynamically created attribute; set automatically
    Dynamic                 = 0x8,
};

class IAttribute
{
public:
    virtual ~IAttribute() = default;

    template<typename T, typename E> using ValueFn = std::function<T(E)>;

    virtual IAttribute& setIntValueFn(ValueFn<int, NodeId> valueFn) = 0;
    virtual IAttribute& setIntValueFn(ValueFn<int, EdgeId> valueFn) = 0;
    virtual IAttribute& setIntValueFn(ValueFn<int, const IGraphComponent&> valueFn) = 0;

    virtual IAttribute& setFloatValueFn(ValueFn<double, NodeId> valueFn) = 0;
    virtual IAttribute& setFloatValueFn(ValueFn<double, EdgeId> valueFn) = 0;
    virtual IAttribute& setFloatValueFn(ValueFn<double, const IGraphComponent&> valueFn) = 0;

    virtual IAttribute& setStringValueFn(ValueFn<QString, NodeId> valueFn) = 0;
    virtual IAttribute& setStringValueFn(ValueFn<QString, EdgeId> valueFn) = 0;
    virtual IAttribute& setStringValueFn(ValueFn<QString, const IGraphComponent&> valueFn) = 0;

    virtual IAttributeRange<int>& intRange() = 0;
    virtual IAttributeRange<double>& floatRange() = 0;
    virtual const IAttributeRange<double>& numericRange() const = 0;

    virtual bool testFlag(AttributeFlag flag) const = 0;
    virtual IAttribute& setFlag(AttributeFlag flag) = 0;
    virtual IAttribute& resetFlag(AttributeFlag flag) = 0;

    virtual bool searchable() const = 0;
    virtual IAttribute& setSearchable(bool searchable) = 0;

    virtual QString description() const = 0;
    virtual IAttribute& setDescription(const QString& description) = 0;
};

#endif // IATTRIBUTE_H
