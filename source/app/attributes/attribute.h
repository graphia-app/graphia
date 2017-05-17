#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include "shared/graph/elementid.h"
#include "shared/graph/elementtype.h"
#include "graph/elementiddistinctsetcollection.h"
#include "shared/attributes/iattribute.h"
#include "shared/graph/igraphcomponent.h"
#include "shared/utils/flags.h"

#include "shared/attributes/valuetype.h"

#include <functional>
#include <limits>
#include <vector>
#include <tuple>
#include <map>

#include <QString>
#include <QRegularExpression>

class Attribute;

template<typename T> class AttributeRange
{};

class _AttributeRange
{
protected:
    Attribute* _attribute = nullptr;

public:
    void setAttribute(Attribute& attribute) { _attribute = &attribute; }
};

template<>
class AttributeRange<int> :
        public IAttributeRange<int>,
        public _AttributeRange
{
public:
    using _AttributeRange::_AttributeRange;

    bool hasMin() const;
    bool hasMax() const;
    bool hasRange() const;

    int min() const;
    int max() const;
    IAttribute& setMin(int min);
    IAttribute& setMax(int max);
};

template<>
class AttributeRange<double> :
        public IAttributeRange<double>,
        public _AttributeRange
{
public:
    using _AttributeRange::_AttributeRange;

    bool hasMin() const;
    bool hasMax() const;
    bool hasRange() const;

    double min() const;
    double max() const;
    IAttribute& setMin(double min);
    IAttribute& setMax(double max);
};

class AttributeNumericRange :
        public IAttributeRange<double>,
        public _AttributeRange
{
public:
    using _AttributeRange::_AttributeRange;

    bool hasMin() const;
    bool hasMax() const;
    bool hasRange() const;

    double min() const;
    double max() const;

    IAttribute& setMin(double min);
    IAttribute& setMax(double max);
};

class Attribute : public IAttribute
{
    friend class AttributeRange<int>;
    friend class AttributeRange<double>;
    friend class AttributeNumericRange;

private:
    // Wrap most of the data members in a struct to make it easier to
    // write/maintain a copy constructor
    struct
    {
        ValueFn<int, NodeId> intNodeIdFn;
        ValueFn<int, EdgeId> intEdgeIdFn;
        ValueFn<int, const IGraphComponent&> intComponentFn;

        ValueFn<double, NodeId> floatNodeIdFn;
        ValueFn<double, EdgeId> floatEdgeIdFn;
        ValueFn<double, const IGraphComponent&> floatComponentFn;

        ValueFn<QString, NodeId> stringNodeIdFn;
        ValueFn<QString, EdgeId> stringEdgeIdFn;
        ValueFn<QString, const IGraphComponent&> stringComponentFn;

        ValueFn<bool, NodeId> valueMissingNodeIdFn;
        ValueFn<bool, EdgeId> valueMissingEdgeIdFn;
        ValueFn<bool, const IGraphComponent&> valueMissingComponentFn;

        int intMin = std::numeric_limits<int>::max();
        int intMax = std::numeric_limits<int>::min();
        double floatMin = std::numeric_limits<double>::max();
        double floatMax = std::numeric_limits<double>::min();

        bool isValid = false;
        Flags<AttributeFlag> flags = AttributeFlag::None;
        bool searchable = false;
        QString description;
    } _;

    AttributeRange<int> _intRange;
    AttributeRange<double> _floatRange;
    AttributeNumericRange _numericRange;

    void clearValueFunctions();
    void clearMissingFunctions();

    template<typename T> struct Helper {};

    int valueOf(Helper<int>, NodeId nodeId) const;
    int valueOf(Helper<int>, EdgeId edgeId) const;
    int valueOf(Helper<int>, const IGraphComponent& component) const;

    double valueOf(Helper<double>, NodeId nodeId) const;
    double valueOf(Helper<double>, EdgeId edgeId) const;
    double valueOf(Helper<double>, const IGraphComponent& component) const;

    QString valueOf(Helper<QString>, NodeId nodeId) const;
    QString valueOf(Helper<QString>, EdgeId edgeId) const;
    QString valueOf(Helper<QString>, const IGraphComponent& component) const;

    bool valueMissingOf(NodeId nodeId) const;
    bool valueMissingOf(EdgeId edgeId) const;
    bool valueMissingOf(const IGraphComponent& component) const;

    enum class Type
    {
        Unknown,
        IntNode,
        IntEdge,
        IntComponent,
        FloatNode,
        FloatEdge,
        FloatComponent,
        StringNode,
        StringEdge,
        StringComponent
    };

    Type type() const;

    void disableAutoRange();

public:
    Attribute()
    {
        _intRange.setAttribute(*this);
        _floatRange.setAttribute(*this);
        _numericRange.setAttribute(*this);
    }

    Attribute(const Attribute& other) :
        _(other._),
        _intRange(other._intRange),
        _floatRange(other._floatRange),
        _numericRange(other._numericRange)
    {
        _intRange.setAttribute(*this);
        _floatRange.setAttribute(*this);
        _numericRange.setAttribute(*this);
    }

    template<typename T, typename E> T valueOf(E& elementId) const
    {
        return valueOf(Helper<T>(), elementId);
    }

    template<typename E> QString stringValueOf(E& elementId) const
    {
        switch(valueType())
        {
        case ValueType::Int:    return QString::number(valueOf<int>(elementId));
        case ValueType::Float:  return QString::number(valueOf<double>(elementId));
        case ValueType::String: return valueOf<QString>(elementId);
        default: break;
        }

        return {};
    }

    template<typename E> double numericValueOf(E& elementId) const
    {
        switch(valueType())
        {
        case ValueType::Int:    return static_cast<double>(valueOf<int>(elementId));
        case ValueType::Float:  return valueOf<double>(elementId);
        default: break;
        }

        return std::numeric_limits<double>::signaling_NaN();
    }

    template<typename E> bool valueMissingOf(E& elementId) const
    {
        return valueMissingOf(elementId);
    }

    bool hasMissingValues() const;

    template<typename T, typename E>
    using ValueOfFn = T(Attribute::*)(E&) const;

    Attribute& setIntValueFn(ValueFn<int, NodeId> valueFn);
    Attribute& setIntValueFn(ValueFn<int, EdgeId> valueFn);
    Attribute& setIntValueFn(ValueFn<int, const IGraphComponent&> valueFn);

    Attribute& setFloatValueFn(ValueFn<double, NodeId> valueFn);
    Attribute& setFloatValueFn(ValueFn<double, EdgeId> valueFn);
    Attribute& setFloatValueFn(ValueFn<double, const IGraphComponent&> valueFn);

    Attribute& setStringValueFn(ValueFn<QString, NodeId> valueFn);
    Attribute& setStringValueFn(ValueFn<QString, EdgeId> valueFn);
    Attribute& setStringValueFn(ValueFn<QString, const IGraphComponent&> valueFn);

    Attribute& setValueMissingFn(ValueFn<bool, NodeId> missingFn);
    Attribute& setValueMissingFn(ValueFn<bool, EdgeId> missingFn);
    Attribute& setValueMissingFn(ValueFn<bool, const IGraphComponent&> missingFn);

    ValueType valueType() const;
    ElementType elementType() const;

    template<typename T>
    AttributeRange<T> range()
    {
        AttributeRange<T> r;
        r.setAttribute(*this);

        return r;
    }

    IAttributeRange<int>& intRange() { return _intRange; }
    IAttributeRange<double>& floatRange() { return _floatRange; }
    const IAttributeRange<double>& numericRange() const { return _numericRange; }

    template<typename T, typename E, typename Fn>
    auto findRangeforElements(const std::vector<E>& elementIds, Fn&& skip) const
    {
        std::tuple<T, T> minMax(std::numeric_limits<T>::max(),
                                std::numeric_limits<T>::min());

        for(auto elementId : elementIds)
        {
            if(skip(*this, elementId))
                continue;

            auto v = valueOf<T>(elementId);
            std::get<0>(minMax) = std::min(v, std::get<0>(minMax));
            std::get<1>(minMax) = std::max(v, std::get<1>(minMax));
        }

        return minMax;
    }

    template<typename T, typename E>
    auto findRangeforElements(const std::vector<E>& elementIds) const
    {
        return findRangeforElements<T>(elementIds, [](const Attribute&, E){ return false; });
    }

    template<typename E, typename Fn>
    auto findRangeforElements(const std::vector<E>& elementIds, Fn&& skip) const
    {
        std::tuple<double, double> minMax(std::numeric_limits<double>::max(),
                                          std::numeric_limits<double>::min());

        if(valueType() == ValueType::Float)
            minMax = findRangeforElements<double>(elementIds, skip);
        else if(valueType() == ValueType::Int)
        {
            auto intMinMax = findRangeforElements<int>(elementIds, skip);
            std::get<0>(minMax) = static_cast<double>(std::get<0>(intMinMax));
            std::get<1>(minMax) = static_cast<double>(std::get<1>(intMinMax));
        }

        return minMax;
    }

    template<typename E>
    auto findRangeforElements(const std::vector<E>& elementIds) const
    {
        return findRangeforElements(elementIds, [](const Attribute&, E){ return false; });
    }

    template<typename T, typename E>
    void autoSetRangeForElements(const std::vector<E>& elementIds)
    {
        auto previousFlags = flags();
        auto minMax = findRangeforElements<T>(elementIds);

        range<T>().setMin(std::get<0>(minMax));
        range<T>().setMax(std::get<1>(minMax));

        setFlag(previousFlags);
    }

    template<typename E>
    void autoSetRangeForElements(const std::vector<E>& elementIds)
    {
        if(valueType() == ValueType::Float)
            autoSetRangeForElements<double>(elementIds);
        else if(valueType() == ValueType::Int)
            autoSetRangeForElements<int>(elementIds);
    }

    AttributeFlag flags() const { return *_.flags; }
    bool testFlag(AttributeFlag flag) const { return _.flags.test(flag); }
    Attribute& setFlag(AttributeFlag flag) { _.flags.set(flag); return *this; }
    Attribute& resetFlag(AttributeFlag flag) { _.flags.reset(flag); return *this; }

    bool searchable() const { return _.searchable; }
    Attribute& setSearchable(bool searchable) { _.searchable = searchable; return *this; }

    QString description() const { return _.description; }
    Attribute& setDescription(const QString& description) { _.description = description; return *this; }

    bool isValid() const;

    enum class EdgeNodeType { None, Source, Target };
    struct Name
    {
        EdgeNodeType _type;
        QString _name;
    };

    static Name parseAttributeName(const QString& name);
    static Attribute edgeNodesAttribute(const IGraph& graph, const Attribute& nodeAttribute,
                                        EdgeNodeType edgeNodeType);
};

#endif // ATTRIBUTE_H
