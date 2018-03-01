#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include "shared/graph/elementid.h"
#include "shared/graph/elementtype.h"
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
#include <QCollator>

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

    bool hasMin() const override;
    bool hasMax() const override;
    bool hasRange() const override;

    int min() const override;
    int max() const override;
    IAttribute& setMin(int min) override;
    IAttribute& setMax(int max) override;
};

template<>
class AttributeRange<double> :
        public IAttributeRange<double>,
        public _AttributeRange
{
public:
    using _AttributeRange::_AttributeRange;

    bool hasMin() const override;
    bool hasMax() const override;
    bool hasRange() const override;

    double min() const override;
    double max() const override;
    IAttribute& setMin(double min) override;
    IAttribute& setMax(double max) override;
};

class AttributeNumericRange :
        public IAttributeRange<double>,
        public _AttributeRange
{
public:
    using _AttributeRange::_AttributeRange;

    bool hasMin() const override;
    bool hasMax() const override;
    bool hasRange() const override;

    double min() const override;
    double max() const override;

    IAttribute& setMin(double min) override;
    IAttribute& setMax(double max) override;
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
        int intMax = std::numeric_limits<int>::lowest();
        double floatMin = std::numeric_limits<double>::max();
        double floatMax = std::numeric_limits<double>::lowest();

        std::vector<SharedValue> _sharedValues;

        bool isValid = false;
        Flags<AttributeFlag> flags = AttributeFlag::None;
        bool searchable = false;

        // Set if the attribute is derived from user data, i.e. not calculated
        bool userDefined = false;

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

    template<typename E> int intValueOf(E& elementId) const
    {
        switch(valueType())
        {
        case ValueType::Int:    return valueOf<int>(elementId);
        case ValueType::Float:  return static_cast<int>(valueOf<double>(elementId));
        case ValueType::String: return valueOf<QString>(elementId).toInt();
        default: break;
        }

        return {};
    }

    template<typename E> double floatValueOf(E& elementId) const
    {
        switch(valueType())
        {
        case ValueType::Int:    return static_cast<double>(valueOf<int>(elementId));
        case ValueType::Float:  return valueOf<double>(elementId);
        case ValueType::String: return valueOf<QString>(elementId).toDouble();
        default: break;
        }

        return {};
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

    bool valueMissingOf(NodeId nodeId) const override;
    bool valueMissingOf(EdgeId edgeId) const override;
    bool valueMissingOf(const IGraphComponent& component) const override;

    template<typename E> bool valueMissingOf(E& elementId) const
    {
        return valueMissingOf(elementId);
    }

    bool hasMissingValues() const;

    template<typename T, typename E>
    using ValueOfFn = T(Attribute::*)(E&) const;

    int intValueOf(NodeId nodeId) const override { return intValueOf<NodeId>(nodeId); }
    int intValueOf(EdgeId edgeId) const override { return intValueOf<EdgeId>(edgeId); }
    int intValueOf(const IGraphComponent& graphComponent) const override
    {
        return intValueOf<const IGraphComponent&>(graphComponent);
    }

    double floatValueOf(NodeId nodeId) const override { return floatValueOf<NodeId>(nodeId); }
    double floatValueOf(EdgeId edgeId) const override { return floatValueOf<EdgeId>(edgeId); }
    double floatValueOf(const IGraphComponent& graphComponent) const override
    {
        return floatValueOf<const IGraphComponent&>(graphComponent);
    }

    QString stringValueOf(NodeId nodeId) const override { return stringValueOf<NodeId>(nodeId); }
    QString stringValueOf(EdgeId edgeId) const override { return stringValueOf<EdgeId>(edgeId); }
    QString stringValueOf(const IGraphComponent& graphComponent) const override
    {
        return stringValueOf<const IGraphComponent&>(graphComponent);
    }

    Attribute& setIntValueFn(ValueFn<int, NodeId> valueFn) override;
    Attribute& setIntValueFn(ValueFn<int, EdgeId> valueFn) override;
    Attribute& setIntValueFn(ValueFn<int, const IGraphComponent&> valueFn) override;

    Attribute& setFloatValueFn(ValueFn<double, NodeId> valueFn) override;
    Attribute& setFloatValueFn(ValueFn<double, EdgeId> valueFn) override;
    Attribute& setFloatValueFn(ValueFn<double, const IGraphComponent&> valueFn) override;

    Attribute& setStringValueFn(ValueFn<QString, NodeId> valueFn) override;
    Attribute& setStringValueFn(ValueFn<QString, EdgeId> valueFn) override;
    Attribute& setStringValueFn(ValueFn<QString, const IGraphComponent&> valueFn) override;

    Attribute& setValueMissingFn(ValueFn<bool, NodeId> missingFn) override;
    Attribute& setValueMissingFn(ValueFn<bool, EdgeId> missingFn) override;
    Attribute& setValueMissingFn(ValueFn<bool, const IGraphComponent&> missingFn) override;

    ValueType valueType() const override;
    ElementType elementType() const override;

    template<typename T>
    AttributeRange<T> range()
    {
        AttributeRange<T> r;
        r.setAttribute(*this);

        return r;
    }

    IAttributeRange<int>& intRange() override { return _intRange; }
    IAttributeRange<double>& floatRange() override { return _floatRange; }
    const IAttributeRange<double>& numericRange() const override { return _numericRange; }

    template<typename E>
    void updateSharedValuesForElements(const std::vector<E>& elementIds)
    {
        _._sharedValues.clear();

        bool hasSharedValues = false;
        std::map<QString, int> values;

        for(auto elementId : elementIds)
        {
            auto value = stringValueOf(elementId);

            if(!value.isEmpty())
            {
                int numValues = values[value]++;
                if(numValues > 1)
                    hasSharedValues = true;
            }
        }

        // Every single value observed is unique
        if(!hasSharedValues)
            return;

        for(auto& value : values)
            _._sharedValues.push_back({value.first, value.second});

        // Sort in reverse order of how often the value occurs
        QCollator collator;
        collator.setNumericMode(true);
        std::sort(_._sharedValues.begin(), _._sharedValues.end(),
        [collator = std::move(collator)](const auto& a, const auto& b)
        {
            if(a._count == b._count)
                return collator.compare(a._value, b._value) < 0;

            return a._count > b._count;
        });
    }

    template<typename T, typename E, typename Fn>
    auto findRangeforElements(const std::vector<E>& elementIds, Fn&& skip) const
    {
        std::tuple<T, T> minMax(std::numeric_limits<T>::max(),
                                std::numeric_limits<T>::lowest());

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
                                          std::numeric_limits<double>::lowest());

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
    bool testFlag(AttributeFlag flag) const override { return _.flags.test(flag); }
    Attribute& setFlag(AttributeFlag flag) override { _.flags.set(flag); return *this; }
    Attribute& resetFlag(AttributeFlag flag) override { _.flags.reset(flag); return *this; }

    std::vector<SharedValue> sharedValues() const { return _._sharedValues; }

    bool searchable() const override { return _.searchable; }
    Attribute& setSearchable(bool searchable) override { _.searchable = searchable; return *this; }

    bool userDefined() const override { return _.userDefined; }
    IAttribute& setUserDefined(bool userDefined) override { _.userDefined = userDefined; return *this; }

    QString description() const override { return _.description; }
    Attribute& setDescription(const QString& description) override { _.description = description; return *this; }

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
