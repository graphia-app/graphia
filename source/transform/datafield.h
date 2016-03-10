#ifndef DATAFIELD_H
#define DATAFIELD_H

#include "../graph/elementid.h"

#include "../utils/enumreflection.h"

#include <functional>
#include <limits>
#include <vector>

#include <QString>

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

enum class DataFieldElementType
{
    Unknown,
    Node,
    Edge,
    Component
};

class DataField
{
public:
    template<typename T, typename E> using ValueFn = std::function<T(E)>;

private:
    ValueFn<int, NodeId> _intNodeIdFn;
    ValueFn<int, EdgeId> _intEdgeIdFn;
    ValueFn<int, const GraphComponent&> _intComponentFn;

    ValueFn<float, NodeId> _floatNodeIdFn;
    ValueFn<float, EdgeId> _floatEdgeIdFn;
    ValueFn<float, const GraphComponent&> _floatComponentFn;

    ValueFn<QString, NodeId> _stringNodeIdFn;
    ValueFn<QString, EdgeId> _stringEdgeIdFn;
    ValueFn<QString, const GraphComponent&> _stringComponentFn;

    void clearFunctions();

    int _intMin = std::numeric_limits<int>::max();
    int _intMax = std::numeric_limits<int>::min();

    float _floatMin = std::numeric_limits<float>::max();
    float _floatMax = std::numeric_limits<float>::min();

    template<typename T> struct Helper {};

    int valueOf(Helper<int>, NodeId nodeId) const;
    int valueOf(Helper<int>, EdgeId edgeId) const;
    int valueOf(Helper<int>, const GraphComponent& component) const;

    float valueOf(Helper<float>, NodeId nodeId) const;
    float valueOf(Helper<float>, EdgeId edgeId) const;
    float valueOf(Helper<float>, const GraphComponent& component) const;

    QString valueOf(Helper<QString>, NodeId nodeId) const;
    QString valueOf(Helper<QString>, EdgeId edgeId) const;
    QString valueOf(Helper<QString>, const GraphComponent& component) const;

    template<typename T, typename E> T valueOf(E elementId) const
    {
        return valueOf(Helper<T>(), elementId);
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
            return [this, value](E elementId) { return valueOf<QString>(elementId).contains(value); };
        case ConditionFnOp::DoesntContain:
            return [this, value](E elementId) { return !valueOf<QString>(elementId).contains(value); };
        case ConditionFnOp::StartsWith:
            return [this, value](E elementId) { return valueOf<QString>(elementId).startsWith(value); };
        case ConditionFnOp::EndsWith:
            return [this, value](E elementId) { return valueOf<QString>(elementId).endsWith(value); };
        case ConditionFnOp::MatchesRegex:
            return [this, value](E elementId) { return valueOf<QString>(elementId).contains(QRegExp(value)); };
        default:
            return createConditionFn<QString, E>(Helper<E>(), op, value);
        }
    }

    template<typename E> ElementConditionFn<E> createConditionFn(Helper<E>, ConditionFnOp op, const char* value) const
    {
        return createConditionFn(Helper<E>(), op, QString(value));
    }

public:
    DataField& setIntValueFn(ValueFn<int, NodeId> valueFn);
    DataField& setIntValueFn(ValueFn<int, EdgeId> valueFn);
    DataField& setIntValueFn(ValueFn<int, const GraphComponent&> valueFn);

    DataField& setFloatValueFn(ValueFn<float, NodeId> valueFn);
    DataField& setFloatValueFn(ValueFn<float, EdgeId> valueFn);
    DataField& setFloatValueFn(ValueFn<float, const GraphComponent&> valueFn);

    DataField& setStringValueFn(ValueFn<QString, NodeId> valueFn);
    DataField& setStringValueFn(ValueFn<QString, EdgeId> valueFn);
    DataField& setStringValueFn(ValueFn<QString, const GraphComponent&> valueFn);

// This may go away with C++14
#if defined(_MSC_VER)
#if _MSC_VER <= 1900
#define INT_NODE_FN(x) DataField::ValueFn<int, NodeId>(x)
#define STRING_NODE_FN(x) DataField::ValueFn<QString, NodeId>(x)
#define FLOAT_EDGE_FN(x) DataField::ValueFn<float, EdgeId>(x)
#define INT_COMPONENT_FN(x) DataField::ValueFn<int, const GraphComponent&>(x)
#else
#error This hack can hopefully be removed now
#endif
#else
#define INT_NODE_FN(x) (x)
#define STRING_NODE_FN(x) (x)
#define FLOAT_EDGE_FN(x) (x)
#define INT_COMPONENT_FN(x) (x)
#endif

    DataFieldType type() const;
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
        return createConditionFn(Helper<const GraphComponent&>(), op, value);
    }
};

#endif // DATAFIELD_H

