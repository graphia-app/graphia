#ifndef DATAFIELD_H
#define DATAFIELD_H

#include "shared/graph/elementid.h"
#include "shared/transform/idatafield.h"
#include "shared/graph/igraphcomponent.h"
#include "utils/qmlenum.h"

#include <functional>
#include <limits>
#include <vector>

#include <QString>
#include <QRegularExpression>

DEFINE_QML_ENUM(Q_GADGET, FieldType,
                Unknown, Int, Float, String);

class DataField : public IDataField
{
private:
    ValueFn<int, NodeId> _intNodeIdFn;
    ValueFn<int, EdgeId> _intEdgeIdFn;
    ValueFn<int, const IGraphComponent&> _intComponentFn;

    ValueFn<double, NodeId> _floatNodeIdFn;
    ValueFn<double, EdgeId> _floatEdgeIdFn;
    ValueFn<double, const IGraphComponent&> _floatComponentFn;

    ValueFn<QString, NodeId> _stringNodeIdFn;
    ValueFn<QString, EdgeId> _stringEdgeIdFn;
    ValueFn<QString, const IGraphComponent&> _stringComponentFn;

    void clearFunctions();

    int _intMin = std::numeric_limits<int>::max();
    int _intMax = std::numeric_limits<int>::min();

    double _floatMin = std::numeric_limits<double>::max();
    double _floatMax = std::numeric_limits<double>::min();

    bool _searchable = false;

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

public:
    template<typename T, typename E> T valueOf(E& elementId) const
    {
        return valueOf(Helper<T>(), elementId);
    }

    template<typename E> QString stringValueOf(E& elementId) const
    {
        switch(valueType())
        {
        case FieldType::Int:    return QString::number(valueOf<int>(elementId));
        case FieldType::Float:  return QString::number(valueOf<double>(elementId));
        case FieldType::String: return valueOf<QString>(elementId);
        default: break;
        }

        return {};
    }

    template<typename T, typename E>
    using ValueOfFn = T(DataField::*)(E&) const;

    DataField& setIntValueFn(ValueFn<int, NodeId> valueFn);
    DataField& setIntValueFn(ValueFn<int, EdgeId> valueFn);
    DataField& setIntValueFn(ValueFn<int, const IGraphComponent&> valueFn);

    DataField& setFloatValueFn(ValueFn<double, NodeId> valueFn);
    DataField& setFloatValueFn(ValueFn<double, EdgeId> valueFn);
    DataField& setFloatValueFn(ValueFn<double, const IGraphComponent&> valueFn);

    DataField& setStringValueFn(ValueFn<QString, NodeId> valueFn);
    DataField& setStringValueFn(ValueFn<QString, EdgeId> valueFn);
    DataField& setStringValueFn(ValueFn<QString, const IGraphComponent&> valueFn);

    FieldType valueType() const;
    ElementType elementType() const;

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

    double floatMin() const;
    double floatMax() const;
    DataField& setFloatMin(double floatMin);
    DataField& setFloatMax(double floatMax);

    bool floatValueInRange(double value) const;

    bool searchable() const { return _searchable; }
    DataField& setSearchable(bool searchable) { _searchable = searchable; return *this; }
};

#endif // DATAFIELD_H

