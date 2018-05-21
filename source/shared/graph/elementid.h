#ifndef ELEMENTID_H
#define ELEMENTID_H

#include <cassert>

template<typename T> class ElementId
{
private:
    static const int NullValue = -1;
    int _value;

public:
    ElementId(int value = NullValue) : // NOLINT
        _value(value)
    {
        static_assert(sizeof(ElementId) == sizeof(_value), "ElementId should not be larger than an int");
    }

    explicit operator int() const { return _value; }
    ElementId& operator=(const ElementId<T>& other) = default;
    T& operator++() { ++_value; return static_cast<T&>(*this); }
    T operator++(int) { T previous = static_cast<T&>(*this); ++_value; return previous; }
    T& operator--() { --_value; return static_cast<T&>(*this); }
    T operator--(int) { T previous = static_cast<T&>(*this); --_value; return previous; }
    bool operator==(const ElementId<T>& other) const { return _value == other._value; }
    bool operator!=(const ElementId<T>& other) const { return _value != other._value; }
    bool operator<(const ElementId<T>& other) const { return _value < other._value; }
    bool operator<=(const ElementId<T>& other) const { return _value <= other._value; }
    bool operator>(const ElementId<T>& other) const { return _value > other._value; }
    bool operator>=(const ElementId<T>& other) const { return _value >= other._value; }
    T operator+(int value) const { return _value + value; }
    T operator-(int value) const { return _value - value; }

    bool isNull() const { assert(_value >= NullValue); return _value == NullValue; }
    void setToNull() { _value = NullValue; }
};

class NodeId :      public ElementId<NodeId>      { using ElementId::ElementId; };
class EdgeId :      public ElementId<EdgeId>      { using ElementId::ElementId; };
class ComponentId : public ElementId<ComponentId> { using ElementId::ElementId; };

#endif // ELEMENTID_H
