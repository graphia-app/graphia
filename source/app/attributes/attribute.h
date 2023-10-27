/* Copyright © 2013-2023 Graphia Technologies Ltd.
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

#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include "shared/graph/elementid.h"
#include "shared/graph/elementtype.h"
#include "shared/attributes/iattribute.h"
#include "shared/graph/igraphcomponent.h"
#include "shared/utils/flags.h"
#include "shared/utils/statistics.h"

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
    Attribute* _attribute = nullptr; // NOLINT cppcoreguidelines-non-private-member-variables-in-classes

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

        SetValueFn<NodeId> setValueNodeIdFn;
        SetValueFn<EdgeId> setValueEdgeIdFn;
        SetValueFn<const IGraphComponent&> setValueComponentFn;

        int intMin = std::numeric_limits<int>::max();
        int intMax = std::numeric_limits<int>::lowest();
        double floatMin = std::numeric_limits<double>::max();
        double floatMax = std::numeric_limits<double>::lowest();

        std::vector<SharedValue> sharedValues;

        Flags<AttributeFlag> flags = AttributeFlag::None;

        // Set if the attribute is derived from user data, i.e. not calculated
        bool userDefined = false;
        MetaDataFn metaDataFn = []{ return QVariantMap(); };

        int parameterIndex = -1;
        QStringList validParameterValues;

        QString description;
    } _;

    AttributeRange<int> _intRange;
    AttributeRange<double> _floatRange;
    AttributeNumericRange _numericRange;

    void clearValueFunctions();
    void clearMissingFunctions();
    void clearSetValueFunctions();

    template<typename T, typename E>
    bool valueFnIsSet(const ValueFn<T, E>& valueFn) const
    {
        return valueFn.index() != 0;
    }

    template<typename T, typename E>
    T callValueFn(const ValueFn<T, E>& valueFn, E& elementId) const
    {
        struct Visitor
        {
            E _elementId; // NOLINT cppcoreguidelines-avoid-const-or-ref-data-members
            const IAttribute* _attribute;

            explicit Visitor(E elementId_, const IAttribute* attribute) :
                _elementId(elementId_), _attribute(attribute) {}

            T operator()(void*) const { qFatal("valueFn is null"); return {}; }

            T operator()(const std::function<T(E)>& valueFn) const
            {
                return valueFn(_elementId);
            }

            T operator()(const std::function<T(E, const IAttribute&)>& valueFn) const
            {
                return valueFn(_elementId, *_attribute);
            }
        };

        return std::visit(Visitor(elementId, this), valueFn);
    }

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
    Attribute() // NOLINT modernize-use-equals-default
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

    Attribute& operator=(const Attribute& other)
    {
        if(this == &other)
            return *this;

        _ = other._;
        _intRange = other._intRange;
        _floatRange = other._floatRange;
        _numericRange = other._numericRange;

        _intRange.setAttribute(*this);
        _floatRange.setAttribute(*this);
        _numericRange.setAttribute(*this);

        return *this;
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

    double numericValueOf(NodeId nodeId) const override { return numericValueOf<NodeId>(nodeId); }
    double numericValueOf(EdgeId edgeId) const override { return numericValueOf<EdgeId>(edgeId); }
    double numericValueOf(const IGraphComponent& graphComponent) const override
    {
        return numericValueOf<const IGraphComponent&>(graphComponent);
    }

    QString stringValueOf(NodeId nodeId) const override { return stringValueOf<NodeId>(nodeId); }
    QString stringValueOf(EdgeId edgeId) const override { return stringValueOf<EdgeId>(edgeId); }
    QString stringValueOf(const IGraphComponent& graphComponent) const override
    {
        return stringValueOf<const IGraphComponent&>(graphComponent);
    }

    void setValueOf(NodeId nodeId, const QString& value) const override;
    void setValueOf(EdgeId edgeId, const QString& value) const override;
    void setValueOf(const IGraphComponent& graphComponent, const QString& value) const override;

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

    Attribute& setSetValueFn(SetValueFn<NodeId> setValueFn) override;
    Attribute& setSetValueFn(SetValueFn<EdgeId> setValueFn) override;
    Attribute& setSetValueFn(SetValueFn<const IGraphComponent&> setValueFn) override;

    ValueType valueType() const override;
    ElementType elementType() const override;

    template<typename E>
    bool isOfElementType() const
    {
        if constexpr(std::is_same_v<E, NodeId>)
            return elementType() == ElementType::Node;

        if constexpr(std::is_same_v<E, EdgeId>)
            return elementType() == ElementType::Edge;

        if constexpr(std::is_same_v<E, const IGraphComponent&>)
            return elementType() == ElementType::Component;
    }

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
    std::vector<SharedValue> findSharedValuesForElements(const std::vector<E>& elementIds,
        bool ignoreIfAllUnique = false) const
    {
        std::vector<SharedValue> result;

        bool hasSharedValues = false;
        std::map<QString, int> values;

        for(auto elementId : elementIds)
        {
            auto value = stringValueOf(elementId);

            if(!value.isEmpty())
            {
                const int numValues = ++values[value];
                if(numValues > 1)
                    hasSharedValues = true;
            }
        }

        // Every single value observed is unique
        if(!hasSharedValues && ignoreIfAllUnique)
            return result;

        result.reserve(values.size());

        std::transform(values.begin(), values.end(), std::back_inserter(result),
        [](const auto& value)
        {
            return SharedValue{value.first, value.second};
        });

        // Sort in reverse order of how often the value occurs
        QCollator collator;
        collator.setNumericMode(true);
        std::sort(result.begin(), result.end(),
        [collator = std::move(collator)](const auto& a, const auto& b)
        {
            if(a._count == b._count)
                return collator.compare(a._value, b._value) < 0;

            return a._count > b._count;
        });

        return result;
    }

    template<typename E>
    void updateSharedValuesForElements(const std::vector<E>& elementIds)
    {
        _.sharedValues = findSharedValuesForElements(elementIds, true);
    }

    template<typename T, typename E>
    auto findRangeforElements(const std::vector<E>& elementIds) const
    {
        std::tuple<T, T> minMax(std::numeric_limits<T>::max(),
                                std::numeric_limits<T>::lowest());

        for(auto elementId : elementIds)
        {
            auto v = valueOf<T>(elementId);
            std::get<0>(minMax) = std::min(v, std::get<0>(minMax));
            std::get<1>(minMax) = std::max(v, std::get<1>(minMax));
        }

        return minMax;
    }

    template<typename E>
    auto findRangeforElements(const std::vector<E>& elementIds) const
    {
        std::tuple<double, double> minMax(std::numeric_limits<double>::max(),
                                          std::numeric_limits<double>::lowest());

        if(valueType() == ValueType::Float)
            minMax = findRangeforElements<double>(elementIds);
        else if(valueType() == ValueType::Int)
        {
            auto intMinMax = findRangeforElements<int>(elementIds);
            std::get<0>(minMax) = static_cast<double>(std::get<0>(intMinMax));
            std::get<1>(minMax) = static_cast<double>(std::get<1>(intMinMax));
        }

        return minMax;
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

    template<typename E>
    u::Statistics findStatisticsforElements(const std::vector<E>& elementIds,
        bool storeValues = false) const
    {
        return u::findStatisticsFor(elementIds,
        [this](const auto& elementId)
        {
            return this->floatValueOf(elementId);
        }, storeValues);
    }

    AttributeFlag flags() const { return *_.flags; }
    bool testFlag(AttributeFlag flag) const override { return _.flags.test(flag); }
    Attribute& setFlag(AttributeFlag flag) override { _.flags.set(flag); return *this; }
    Attribute& resetFlag(AttributeFlag flag) override { _.flags.reset(flag); return *this; }

    std::vector<SharedValue> sharedValues() const override { return _.sharedValues; }

    bool userDefined() const override { return _.userDefined; }
    IAttribute& setUserDefined(bool userDefined) override { _.userDefined = userDefined; return *this; }

    QVariantMap metaData() const override { return _.metaDataFn(); }
    IAttribute& setMetaDataFn(MetaDataFn metaDataFn) override { _.metaDataFn = metaDataFn; return *this; }

    bool editable() const override;

    QString description() const override { return _.description; }
    Attribute& setDescription(const QString& description) override { _.description = description; return *this; }

    bool isValid() const override;

    QString parameterValue() const override;
    bool setParameterValue(const QString& value) override;

    bool hasParameter() const override;
    QStringList validParameterValues() const override;
    IAttribute& setValidParameterValues(const QStringList& values) override;

    enum class EdgeNodeType { None, Source, Target };
    struct Name
    {
        EdgeNodeType _type;
        QString _name;
        QString _parameter;
    };

    static Name parseAttributeName(QString name);
    static QString enquoteAttributeName(const QString& name);
    static Attribute edgeNodesAttribute(const IGraph& graph,
        const Attribute& nodeAttribute, EdgeNodeType edgeNodeType);

    static QString prettify(QString name);
};

#endif // ATTRIBUTE_H
