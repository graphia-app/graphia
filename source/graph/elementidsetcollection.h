#ifndef ELEMENTIDSETCOLLECTION
#define ELEMENTIDSETCOLLECTION

#include "elementid.h"

#include <vector>

template<typename T> class ElementIdSetCollection
{
    static_assert(std::is_base_of<ElementId<T>, T>::value, "T must be an ElementId");

private:
    struct MultiElementId
    {
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
    };

    std::vector<MultiElementId> _multiElementIds;

public:
    enum class Type
    {
        Not,
        Head,
        Tail
    };

    void resize(std::size_t size)
    {
        _multiElementIds.resize(size);
    }

    void clear()
    {
        _multiElementIds.clear();
    }

    T add(T elementIdA, T elementIdB)
    {
        Q_ASSERT(!elementIdA.isNull());
        Q_ASSERT(!elementIdB.isNull());

        T lowId, highId;
        std::tie(lowId, highId) = std::minmax(elementIdA, elementIdB);
        auto& lowMultiId = _multiElementIds[lowId];
        auto& highMultiId = _multiElementIds[highId];

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
            auto& tail = _multiElementIds[highMultiId._opposite];
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
            auto& head = _multiElementIds[lowMultiId._opposite];
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
                auto& next = _multiElementIds[lowMultiId._next];

                highMultiId._prev = lowId;
                highMultiId._next = lowMultiId._next;

                lowMultiId._next = highId;
                next._prev = highId;
            }
            else if(!highMultiId.isNull())
            {
                Q_ASSERT(lowMultiId.isNull());
                Q_ASSERT(!highMultiId._prev.isNull());
                auto& prev = _multiElementIds[highMultiId._prev];

                lowMultiId._prev = highMultiId._prev;
                lowMultiId._next = highId;

                highMultiId._prev = lowId;
                prev._next = lowId;
            }
        }

        return lowId;
    }

    void remove(T elementId)
    {
        Q_ASSERT(!elementId.isNull());
        auto& multiElementId = _multiElementIds[elementId];

        // Can't remove it if it isn't a multielement
        if(multiElementId.isNull())
            return;

        if(multiElementId._next == multiElementId._opposite)
        {
            // The tail is the only other element
            auto& tail = _multiElementIds[multiElementId._next];
            tail.setToSingleton(multiElementId._next);
        }
        else if(multiElementId._prev == multiElementId._opposite)
        {
            // The head is the only other element
            auto& head = _multiElementIds[multiElementId._prev];
            head.setToSingleton(multiElementId._prev);
        }
        else if(multiElementId.isHead(elementId))
        {
            // Removing from the head
            Q_ASSERT(!multiElementId._next.isNull());
            Q_ASSERT(!multiElementId._opposite.isNull());
            auto& newHead = _multiElementIds[multiElementId._next];
            auto& tail = _multiElementIds[multiElementId._opposite];

            newHead._opposite = multiElementId._opposite;
            newHead._prev.setToNull();
            tail._opposite = multiElementId._next;
        }
        else if(multiElementId.isTail(elementId))
        {
            // Removing from the tail
            Q_ASSERT(!multiElementId._opposite.isNull());
            Q_ASSERT(!multiElementId._prev.isNull());
            auto& head = _multiElementIds[multiElementId._opposite];
            auto& newTail = _multiElementIds[multiElementId._prev];

            head._opposite = multiElementId._prev;
            newTail._next = multiElementId._prev;
            newTail._opposite = multiElementId._opposite;
        }
        else
        {
            // Removing in the middle
            Q_ASSERT(!multiElementId._prev.isNull());
            Q_ASSERT(!multiElementId._next.isNull());
            auto& prev = _multiElementIds[multiElementId._prev];
            auto& next = _multiElementIds[multiElementId._next];

            prev._next = multiElementId._next;
            next._prev = multiElementId._prev;
        }

        multiElementId.setToNull();
    }

    Type typeOf(T elementId) const
    {
        Q_ASSERT(!elementId.isNull());
        auto& multiElementId = _multiElementIds[elementId];

        if(!multiElementId.isNull() && !multiElementId.isSingleton(elementId))
        {
            if(multiElementId.isHead(elementId))
                return Type::Head;

            return Type::Tail;
        }

        return Type::Not;
    }

    ElementIdSet<T> elements(T elementId) const
    {
        ElementIdSet<T> elementIdSet;
        elementIdSet.insert(elementId);

        if(typeOf(elementId) == Type::Head)
        {
            const auto* multiElementId = &_multiElementIds[elementId];

            while(multiElementId->hasNext(elementId))
            {
                elementIdSet.insert(multiElementId->_next);
                elementId = multiElementId->_next;
                multiElementId = &_multiElementIds[multiElementId->_next];
            }
        }

        return elementIdSet;
    }
};

using NodeIdSetCollection = ElementIdSetCollection<NodeId>;
using EdgeIdSetCollection = ElementIdSetCollection<EdgeId>;

#endif // ELEMENTIDSETCOLLECTION

