#ifndef ELEMENTID_H
#define ELEMENTID_H

#include "../utils/cpp1x_hacks.h"

#include <QDebug>

#include <unordered_set>
#include <unordered_map>
#include <functional>

class MutableGraph;

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
            return "Null";
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

class NodeId : public ElementId<NodeId>
{
#if __cplusplus >= 201103L
    using ElementId::ElementId;
#else
public:
    NodeId() : ElementId() {}
    NodeId(int value) : ElementId(value) {}
#endif
};

class EdgeId : public ElementId<EdgeId>
{
#if __cplusplus >= 201103L
    using ElementId::ElementId;
#else
public:
    EdgeId() : ElementId() {}
    EdgeId(int value) : ElementId(value) {}
#endif
};

class ComponentId : public ElementId<ComponentId>
{
#if __cplusplus >= 201103L
    using ElementId::ElementId;
#else
public:
    ComponentId() : ElementId() {}
    ComponentId(int value) : ElementId(value) {}
#endif
};

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

template<typename T> class MultiElementId
{
    static_assert(std::is_base_of<ElementId<T>, T>::value, "T must be an ElementId");

private:
    T _prev;
    T _next;
    T _opposite;

    bool isNull() const { return _next.isNull(); }
    bool isTail(T elementId) const { return _next == elementId; }
    bool isHead(T elementId) const { return !_opposite.isNull() && (!isTail(elementId) || _opposite == elementId); }
    bool isSingleton(T elementId) const { return isHead(elementId) && isTail(elementId); }

    void setToNull() { _prev.setToNull(); _next.setToNull(); _opposite.setToNull(); }
    void setToSingleton(T elementId) { _prev = _next = _opposite = elementId; }

    bool hasNext(T elementId) const { return !_next.isNull() && !isTail(elementId); }

public:
    enum class Type
    {
        Not,
        Head,
        Tail
    };

    template<typename ElementId>
    static ElementId add(ElementId elementIdA, ElementId elementIdB,
                         std::vector<MultiElementId<ElementId>>& multiElementIds)
    {
        Q_ASSERT(!elementIdA.isNull());
        Q_ASSERT(!elementIdB.isNull());

        ElementId lowId, highId;
        std::tie(lowId, highId) = std::minmax(elementIdA, elementIdB);
        auto& lowMultiId = multiElementIds[lowId];
        auto& highMultiId = multiElementIds[highId];

        if(lowMultiId.isSingleton(lowId))
            lowMultiId.setToNull();

        if(highMultiId.isSingleton(highId))
            highMultiId.setToNull();

        if(lowMultiId.isNull() && highMultiId.isNull())
        {
            // Neither is yet merged with anything
            lowMultiId._next = highId;
            lowMultiId._opposite = highId;

            highMultiId._prev = lowId;
            highMultiId._next = highId;
            highMultiId._opposite = lowId;
        }
        else if(highMultiId.isHead(highId))
        {
            Q_ASSERT(!highMultiId._opposite.isNull());
            Q_ASSERT(lowMultiId.isNull());
            Q_ASSERT(lowMultiId._prev.isNull());
            Q_ASSERT(lowMultiId._opposite.isNull());

            // Adding to the head
            auto& tail = multiElementIds[highMultiId._opposite];
            tail._opposite = lowId;

            lowMultiId._next = highId;
            lowMultiId._opposite = highMultiId._opposite;

            highMultiId._prev = lowId;
            highMultiId._opposite.setToNull();
        }
        else if(lowMultiId.isTail(lowId))
        {
            Q_ASSERT(!lowMultiId._opposite.isNull());
            Q_ASSERT(highMultiId.isNull());
            Q_ASSERT(highMultiId._prev.isNull());
            Q_ASSERT(highMultiId._opposite.isNull());

            // Adding to the tail
            auto& head = multiElementIds[lowMultiId._opposite];
            head._opposite = highId;

            highMultiId._prev = lowId;
            highMultiId._next = highId;
            highMultiId._opposite = lowMultiId._opposite;

            lowMultiId._next = highId;
            lowMultiId._opposite.setToNull();
        }
        else
        {
            // Adding in the middle
            if(!lowMultiId.isNull())
            {
                Q_ASSERT(highMultiId.isNull());
                Q_ASSERT(!lowMultiId._next.isNull());
                auto& next = multiElementIds[lowMultiId._next];

                highMultiId._prev = lowId;
                highMultiId._next = lowMultiId._next;

                lowMultiId._next = highId;
                next._prev = highId;
            }
            else if(!highMultiId.isNull())
            {
                Q_ASSERT(lowMultiId.isNull());
                Q_ASSERT(!highMultiId._prev.isNull());
                auto& prev = multiElementIds[highMultiId._prev];

                lowMultiId._prev = highMultiId._prev;
                lowMultiId._next = highId;

                highMultiId._prev = lowId;
                prev._next = lowId;
            }
        }

        return lowId;
    }

    template<typename ElementId>
    static ElementId add(ElementId elementId, std::vector<MultiElementId<ElementId>>& multiElementIds)
    {
        return add(elementId, elementId, multiElementIds);
    }

    template<typename ElementId>
    static void remove(ElementId elementId, std::vector<MultiElementId<ElementId>>& multiElementIds)
    {
        Q_ASSERT(!elementId.isNull());
        auto& multiElementId = multiElementIds[elementId];

        // Can't remove it if it isn't a multielement
        if(multiElementId.isNull())
            return;

        if(multiElementId._next == multiElementId._opposite)
        {
            // The tail is the only other element
            auto& tail = multiElementIds[multiElementId._next];
            tail.setToSingleton(multiElementId._next);
        }
        else if(multiElementId._prev == multiElementId._opposite)
        {
            // The head is the only other element
            auto& head = multiElementIds[multiElementId._prev];
            head.setToSingleton(multiElementId._prev);
        }
        else if(multiElementId.isHead(elementId))
        {
            // Removing from the head
            Q_ASSERT(!multiElementId._next.isNull());
            Q_ASSERT(!multiElementId._opposite.isNull());
            auto& newHead = multiElementIds[multiElementId._next];
            auto& tail = multiElementIds[multiElementId._opposite];

            newHead._opposite = multiElementId._opposite;
            newHead._prev.setToNull();
            tail._opposite = multiElementId._next;
        }
        else if(multiElementId.isTail(elementId))
        {
            // Removing from the tail
            Q_ASSERT(!multiElementId._opposite.isNull());
            Q_ASSERT(!multiElementId._prev.isNull());
            auto& head = multiElementIds[multiElementId._opposite];
            auto& newTail = multiElementIds[multiElementId._prev];

            head._opposite = multiElementId._prev;
            newTail._next = multiElementId._prev;
            newTail._opposite = multiElementId._opposite;
        }
        else
        {
            // Removing in the middle
            Q_ASSERT(!multiElementId._prev.isNull());
            Q_ASSERT(!multiElementId._next.isNull());
            auto& prev = multiElementIds[multiElementId._prev];
            auto& next = multiElementIds[multiElementId._next];

            prev._next = multiElementId._next;
            next._prev = multiElementId._prev;
        }

        multiElementId.setToNull();
    }

    template<typename ElementId> static typename MultiElementId<ElementId>::Type
    typeOf(ElementId elementId, const std::vector<MultiElementId<ElementId>>& multiElementIds)
    {
        Q_ASSERT(!elementId.isNull());
        auto& multiElementId = multiElementIds[elementId];

        if(!multiElementId.isNull() && !multiElementId.isSingleton(elementId))
        {
            if(multiElementId.isHead(elementId))
                return MultiElementId<ElementId>::Type::Head;

            return MultiElementId<ElementId>::Type::Tail;
        }

        return MultiElementId<ElementId>::Type::Not;
    }

    template<typename ElementId> static ElementIdSet<ElementId>
    elements(ElementId elementId, const std::vector<MultiElementId<ElementId>>& multiElementIds)
    {
        ElementIdSet<ElementId> elementIdSet;
        elementIdSet.insert(elementId);

        if(typeOf(elementId, multiElementIds) == MultiElementId<ElementId>::Type::Head)
        {
            const auto* multiElementId = &multiElementIds[elementId];

            while(multiElementId->hasNext(elementId))
            {
                elementIdSet.insert(multiElementId->_next);
                elementId = multiElementId->_next;
                multiElementId = &multiElementIds[multiElementId->_next];
            }
        }

        return elementIdSet;
    }
};

using MultiNodeId = MultiElementId<NodeId>;
using MultiEdgeId = MultiElementId<EdgeId>;

#endif // ELEMENTID_H
