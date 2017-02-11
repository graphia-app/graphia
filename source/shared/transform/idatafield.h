#ifndef IDATAFIELD_H
#define IDATAFIELD_H

#include "shared/graph/elementid.h"
#include "shared/graph/igraphcomponent.h"

#include <functional>

class IDataField
{
public:
    virtual ~IDataField() = default;

    template<typename T, typename E> using ValueFn = std::function<T(E)>;

    virtual IDataField& setIntValueFn(ValueFn<int, NodeId> valueFn) = 0;
    virtual IDataField& setIntValueFn(ValueFn<int, EdgeId> valueFn) = 0;
    virtual IDataField& setIntValueFn(ValueFn<int, const IGraphComponent&> valueFn) = 0;

    virtual IDataField& setFloatValueFn(ValueFn<double, NodeId> valueFn) = 0;
    virtual IDataField& setFloatValueFn(ValueFn<double, EdgeId> valueFn) = 0;
    virtual IDataField& setFloatValueFn(ValueFn<double, const IGraphComponent&> valueFn) = 0;

    virtual IDataField& setStringValueFn(ValueFn<QString, NodeId> valueFn) = 0;
    virtual IDataField& setStringValueFn(ValueFn<QString, EdgeId> valueFn) = 0;
    virtual IDataField& setStringValueFn(ValueFn<QString, const IGraphComponent&> valueFn) = 0;

    virtual bool hasIntMin() const = 0;
    virtual bool hasIntMax() const = 0;
    virtual bool hasIntRange() const = 0;

    virtual int intMin() const = 0;
    virtual int intMax() const = 0;
    virtual IDataField& setIntMin(int intMin) = 0;
    virtual IDataField& setIntMax(int intMax) = 0;

    virtual bool intValueInRange(int value) const = 0;

    virtual bool hasFloatMin() const = 0;
    virtual bool hasFloatMax() const = 0;
    virtual bool hasFloatRange() const = 0;

    virtual double floatMin() const = 0;
    virtual double floatMax() const = 0;
    virtual IDataField& setFloatMin(double floatMin) = 0;
    virtual IDataField& setFloatMax(double floatMax) = 0;

    virtual bool floatValueInRange(double value) const = 0;

    virtual bool searchable() const = 0;
    virtual IDataField& setSearchable(bool searchable) = 0;

    virtual QString description() const = 0;
    virtual IDataField& setDescription(const QString& description) = 0;
};

#endif // IDATAFIELD_H
