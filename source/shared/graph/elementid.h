#ifndef ELEMENTID_H
#define ELEMENTID_H

#include <QDebug>
#include <QString>

#include <unordered_set>
#include <unordered_map>
#include <functional>

template<typename T> class ElementId
{
    friend std::hash<ElementId<T>>;
private:
    static const int NullValue = -1;
    int _value;

public:
    ElementId(int value = NullValue) :
        _value(value)
    {
        static_assert(sizeof(ElementId) == sizeof(_value), "ElementId should not be larger than an int");
    }

    inline operator int() const { return _value; }
    ElementId& operator=(const ElementId<T>& other) { _value = other._value; return *this; }
    inline T& operator++() { ++_value; return static_cast<T&>(*this); }
    inline T operator++(int) { T previous = static_cast<T&>(*this); ++_value; return previous; }
    inline T& operator--() { --_value; return static_cast<T&>(*this); }
    inline T operator--(int) { T previous = static_cast<T&>(*this); --_value; return previous; }
    inline bool operator==(const ElementId<T>& other) const { return _value == other._value; }
    inline bool operator==(int value) const { return _value == value; }
    inline bool operator<(const ElementId<T>& other) const { return _value < other._value; }
    inline bool operator<(int value) const { return _value < value; }

    inline bool isNull() const { return _value == NullValue; }
    inline void setToNull() { _value = NullValue; }

    operator QString() const
    {
        if(isNull())
            return QStringLiteral("Null");
        else
            return QString::number(_value);
    }
};

template<typename T> QDebug operator<<(QDebug d, const ElementId<T>& id)
{
    QString idString = id;
    d << idString;

    return d;
}

class NodeId :      public ElementId<NodeId>      { using ElementId::ElementId; };
class EdgeId :      public ElementId<EdgeId>      { using ElementId::ElementId; };
class ComponentId : public ElementId<ComponentId> { using ElementId::ElementId; };

namespace std
{
    template<typename T> struct hash<ElementId<T>>
    {
    public:
        size_t operator()(const ElementId<T>& x) const noexcept
        {
            return x._value;
        }
    };
}

template<typename T> using ElementIdSet = std::unordered_set<T, std::hash<ElementId<T>>>;
template<typename K, typename V> using ElementIdMap = std::unordered_map<K, V, std::hash<ElementId<K>>>;

template<typename T> QDebug operator<<(QDebug d, const ElementIdSet<T>& idSet)
{
    d << "[";
    for(auto id : idSet)
        d << id;
    d << "]";

    return d;
}

using NodeIdSet = ElementIdSet<NodeId>;
using EdgeIdSet = ElementIdSet<EdgeId>;
using ComponentIdSet = ElementIdSet<ComponentId>;

template<typename V> using NodeIdMap = ElementIdMap<NodeId, V>;
template<typename V> using EdgeIdMap = ElementIdMap<EdgeId, V>;
template<typename V> using ComponentIdMap = ElementIdMap<ComponentId, V>;

template<typename Element> using ElementConditionFn = std::function<bool(const Element)>;
using NodeConditionFn = ElementConditionFn<NodeId>;
using EdgeConditionFn = ElementConditionFn<EdgeId>;

class IGraphComponent;
using ComponentConditionFn = std::function<bool(const IGraphComponent& component)>;

#endif // ELEMENTID_H
