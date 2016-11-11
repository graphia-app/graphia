#ifndef DATAFIELD_H
#define DATAFIELD_H

#include "shared/graph/elementid.h"
#include "shared/transform/idatafield.h"
#include "shared/graph/igraphcomponent.h"
#include "shared/utils/enumreflection.h"

#include <functional>
#include <limits>
#include <vector>

#include <QString>
#include <QRegularExpression>

enum class ConditionFnOp
{
    None,
    Equal,
    NotEqual,
    LessThan,
    GreaterThan,
    LessThanOrEqual,
    GreaterThanOrEqual,
    Contains,
    DoesntContain,
    StartsWith,
    EndsWith,
    MatchesRegex
};

DECLARE_REFLECTED_ENUM(ConditionFnOp)

enum class DataFieldType
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

enum class DataFieldValueType
{
    Unknown,
    Int,
    Float,
    String
};

enum class DataFieldElementType
{
    Unknown,
    Node,
    Edge,
    Component
};

class DataField : public IDataField
{
private:
    ValueFn<int, NodeId> _intNodeIdFn;
    ValueFn<int, EdgeId> _intEdgeIdFn;
    ValueFn<int, const IGraphComponent&> _intComponentFn;

    ValueFn<float, NodeId> _floatNodeIdFn;
    ValueFn<float, EdgeId> _floatEdgeIdFn;
    ValueFn<float, const IGraphComponent&> _floatComponentFn;

    ValueFn<QString, NodeId> _stringNodeIdFn;
    ValueFn<QString, EdgeId> _stringEdgeIdFn;
    ValueFn<QString, const IGraphComponent&> _stringComponentFn;

    void clearFunctions();

    int _intMin = std::numeric_limits<int>::max();
    int _intMax = std::numeric_limits<int>::min();

    float _floatMin = std::numeric_limits<float>::max();
    float _floatMax = std::numeric_limits<float>::min();

    bool _searchable = false;

    template<typename T> struct Helper {};

    int valueOf(Helper<int>, NodeId nodeId) const;
    int valueOf(Helper<int>, EdgeId edgeId) const;
    int valueOf(Helper<int>, const IGraphComponent& component) const;

    float valueOf(Helper<float>, NodeId nodeId) const;
    float valueOf(Helper<float>, EdgeId edgeId) const;
    float valueOf(Helper<float>, const IGraphComponent& component) const;

    QString valueOf(Helper<QString>, NodeId nodeId) const;
    QString valueOf(Helper<QString>, EdgeId edgeId) const;
    QString valueOf(Helper<QString>, const IGraphComponent& component) const;

    template<typename T, typename E> T valueOf(E elementId) const
    {
        return valueOf(Helper<T>(), elementId);
    }

    template<typename E> QString stringValueOf(E elementId) const
    {
        switch(valueType())
        {
        case DataFieldValueType::Int:    return QString::number(valueOf<int>(elementId));
        case DataFieldValueType::Float:  return QString::number(valueOf<float>(elementId));
        case DataFieldValueType::String: return valueOf<QString>(elementId);
        default: break;
        }

        return {};
    }

    template<typename T, typename E> ElementConditionFn<E> createConditionFn(Helper<E>, ConditionFnOp op, T value) const
    {
        switch(op)
        {
        case ConditionFnOp::Equal:
            return [this, value](E elementId) { return valueOf<T, E>(elementId) == value; };
        case ConditionFnOp::NotEqual:
            return [this, value](E elementId) { return valueOf<T, E>(elementId) != value; };
        case ConditionFnOp::LessThan:
            return [this, value](E elementId) { return valueOf<T, E>(elementId) < value; };
        case ConditionFnOp::GreaterThan:
            return [this, value](E elementId) { return valueOf<T, E>(elementId) > value; };
        case ConditionFnOp::LessThanOrEqual:
            return [this, value](E elementId) { return valueOf<T, E>(elementId) <= value; };
        case ConditionFnOp::GreaterThanOrEqual:
            return [this, value](E elementId) { return valueOf<T, E>(elementId) >= value; };
        default:
            qFatal("Op is not implemented for T");
            return nullptr;
        }
    }

    template<typename E> ElementConditionFn<E> createConditionFn(Helper<E>, ConditionFnOp op, const QString& value) const
    {
        switch(op)
        {
        case ConditionFnOp::Contains:
            return [this, value](E elementId) { return valueOf<QString, E>(elementId).contains(value); };
        case ConditionFnOp::DoesntContain:
            return [this, value](E elementId) { return !valueOf<QString, E>(elementId).contains(value); };
        case ConditionFnOp::StartsWith:
            return [this, value](E elementId) { return valueOf<QString, E>(elementId).startsWith(value); };
        case ConditionFnOp::EndsWith:
            return [this, value](E elementId) { return valueOf<QString, E>(elementId).endsWith(value); };
        case ConditionFnOp::MatchesRegex:
            return [this, value](E elementId) { return valueOf<QString, E>(elementId).contains(QRegularExpression(value)); };
        default:
            return createConditionFn<QString, E>(Helper<E>(), op, value);
        }
    }

    template<typename E> ElementConditionFn<E> createConditionFn(Helper<E>, ConditionFnOp op, const QRegularExpression& regex) const
    {
        if(op == ConditionFnOp::MatchesRegex)
            return [this, regex](E elementId) { return stringValueOf(elementId).contains(regex); };

        return nullptr;
    }

    template<typename E> ElementConditionFn<E> createConditionFn(Helper<E>, ConditionFnOp op, const char* value) const
    {
        return createConditionFn(Helper<E>(), op, QString(value));
    }

public:
    DataField& setIntValueFn(ValueFn<int, NodeId> valueFn);
    DataField& setIntValueFn(ValueFn<int, EdgeId> valueFn);
    DataField& setIntValueFn(ValueFn<int, const IGraphComponent&> valueFn);

    DataField& setFloatValueFn(ValueFn<float, NodeId> valueFn);
    DataField& setFloatValueFn(ValueFn<float, EdgeId> valueFn);
    DataField& setFloatValueFn(ValueFn<float, const IGraphComponent&> valueFn);

    DataField& setStringValueFn(ValueFn<QString, NodeId> valueFn);
    DataField& setStringValueFn(ValueFn<QString, EdgeId> valueFn);
    DataField& setStringValueFn(ValueFn<QString, const IGraphComponent&> valueFn);

    DataFieldType type() const;
    DataFieldValueType valueType() const;
    DataFieldElementType elementType() const;

    bool hasIntMin() const;
    bool hasIntMax() const;
    bool hasIntRange() const;

    int intMin() const;
    int intMax() const;
    DataField& setIntMin(int intMin);
    DataField& setIntMax(int intMax);

    bool intValueInRange(int value) const;

    bool hasFloatMin() const;
    bool hasFloatMax() const;
    bool hasFloatRange() const;

    float floatMin() const;
    float floatMax() const;
    DataField& setFloatMin(float floatMin);
    DataField& setFloatMax(float floatMax);

    bool floatValueInRange(float value) const;

    DataField& setSearchable(bool searchable) { _searchable = searchable; return *this; }
    bool searchable() const { return _searchable; }

    std::vector<ConditionFnOp> validConditionFnOps() const;

    template<typename T> NodeConditionFn createNodeConditionFn(ConditionFnOp op, T value) const
    {
        return createConditionFn(Helper<NodeId>(), op, value);
    }

    template<typename T> EdgeConditionFn createEdgeConditionFn(ConditionFnOp op, T value) const
    {
        return createConditionFn(Helper<EdgeId>(), op, value);
    }

    template<typename T> ComponentConditionFn createComponentConditionFn(ConditionFnOp op, T value) const
    {
        return createConditionFn(Helper<const IGraphComponent&>(), op, value);
    }
};

#endif // DATAFIELD_H

