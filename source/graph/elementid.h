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
    explicit ElementId(int value = NullValue) :
        _value(value)
    {
        static_assert(sizeof(ElementId) == sizeof(_value), "ElementId should not be larger than an int");
    }

    inline operator int() const { return _value; }
    ElementId& operator=(const ElementId<T>& other) { _value = other._value; return *this; }
    inline T& operator++() { ++_value; return static_cast<T&>(*this); }
    inline T operator++(int) { T previous = static_cast<T&>(*this); ++_value; return previous; }
    inline bool operator==(const ElementId<T>& other) const { return _value == other._value; }
    inline bool operator<(const ElementId<T>& other) const { return _value < other._value; }

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
    explicit NodeId() : ElementId() {}
    explicit NodeId(int value) : ElementId(value) {}
#endif
};

class EdgeId : public ElementId<EdgeId>
{
#if __cplusplus >= 201103L
    using ElementId::ElementId;
#else
public:
    explicit EdgeId() : ElementId() {}
    explicit EdgeId(int value) : ElementId(value) {}
#endif
};

class ComponentId : public ElementId<ComponentId>
{
#if __cplusplus >= 201103L
    using ElementId::ElementId;
#else
public:
    explicit ComponentId() : ElementId() {}
    explicit ComponentId(int value) : ElementId(value) {}
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
private:
    T _next;
    T _id;

    void setToNull() { _id.setToNull(); _next.setToNull(); }
    bool hasNext() const { return !_next.isNull(); }
    T next() const { return _next; }

    template<typename ElementId>
    static void iterateMultiElements(ElementId start, std::vector<MultiElementId<ElementId>>& multiElementIds,
                                     std::function<void(MultiElementId<ElementId>&)> f)

    {
        if(start.isNull())
            return;

        auto* multiElementId = &multiElementIds[start];

        f(*multiElementId);

        while(multiElementId->hasNext())
        {
            multiElementId = &multiElementIds[multiElementId->next()];
            f(*multiElementId);
        }
    }

public:
    enum class Type
    {
        Not,
        Head,
        Tail
    };

    T id() const { return _id; }
    bool isNull() const { return _id.isNull(); }

    template<typename ElementId>
    static ElementId mergeElements(ElementId elementIdA, ElementId elementIdB,
                                   std::vector<MultiElementId<ElementId>>& multiElementIds)
    {
        Q_ASSERT(!elementIdA.isNull());
        Q_ASSERT(!elementIdB.isNull());

        // Can't merge something with itself
        if(elementIdA == elementIdB)
            return ElementId();

        auto setMultiElementId = [&multiElementIds](ElementId start, ElementId elementId)
        {
            iterateMultiElements<ElementId>(start, multiElementIds,
            [&elementId](MultiElementId<ElementId>& multiElementId)
            {
                multiElementId._id = elementId;
            });
        };

        ElementId elementId, nextElementId;
        std::tie(elementId, nextElementId) = std::minmax(elementIdA, elementIdB);
        auto& multiElementId = multiElementIds[elementId];

        multiElementId._next = nextElementId;

        if(multiElementId.isNull())
            setMultiElementId(elementId, elementId);
        else
            setMultiElementId(nextElementId, multiElementId._id);

        return multiElementId._id;
    }

    template<typename ElementId>
    static void removeMultiElementId(ElementId elementId, std::vector<MultiElementId<ElementId>>& multiElementIds)
    {
        Q_ASSERT(!elementId.isNull());

        auto& oldMultiElementId = multiElementIds[elementId];

        if(oldMultiElementId.isNull())
            return;

        auto rightMultiElementId = oldMultiElementId._next;

        iterateMultiElements<ElementId>(oldMultiElementId._id, multiElementIds,
        [elementId, rightMultiElementId](MultiElementId<ElementId>& multiElementId)
        {
            if(multiElementId._next == elementId)
                multiElementId._next.setToNull(); // Break link
            else if(multiElementId._next > rightMultiElementId || multiElementId.isNull())
                multiElementId._id = rightMultiElementId; // Set everything right of removee to new id
        });

        oldMultiElementId.setToNull();
    }

    template<typename ElementId>
    static typename MultiElementId<ElementId>::Type
    typeOf(ElementId elementId, const std::vector<MultiElementId<ElementId>>& multiElementIds)
    {
        auto& multiElementId = multiElementIds[elementId];

        if(!multiElementId.isNull())
        {
            if(multiElementId.id() == elementId)
                return MultiElementId<ElementId>::Type::Head;

            return MultiElementId<ElementId>::Type::Tail;
        }

        return MultiElementId<ElementId>::Type::Not;
    }
};

using MultiNodeId = MultiElementId<NodeId>;
using MultiEdgeId = MultiElementId<EdgeId>;

#endif // ELEMENTID_H
