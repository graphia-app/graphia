#ifndef ELEMENTID_H
#define ELEMENTID_H

namespace std { template<typename> struct hash; }

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
};

class NodeId :      public ElementId<NodeId>      { using ElementId::ElementId; };
class EdgeId :      public ElementId<EdgeId>      { using ElementId::ElementId; };
class ComponentId : public ElementId<ComponentId> { using ElementId::ElementId; };

#endif // ELEMENTID_H
