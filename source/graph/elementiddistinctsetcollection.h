#ifndef ELEMENTIDDISTINCTSETCOLLECTION
#define ELEMENTIDDISTINCTSETCOLLECTION

#include "elementid.h"
#include "../utils/utils.h"

#include <QDebug>

#include <vector>
#include <algorithm>

template<typename C, typename T> class ElementIdDistinctSet;
template<typename T> class ElementIdDistinctSets;

template<typename T> class ElementIdDistinctSetCollection
{
    static_assert(std::is_base_of<ElementId<T>, T>::value, "T must be an ElementId");

    friend class ElementIdDistinctSet<ElementIdDistinctSetCollection<T>, T>;
    friend class ElementIdDistinctSet<const ElementIdDistinctSetCollection<T>, T>;
    friend class ElementIdDistinctSets<T>;

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

    using MultiElementIds = std::vector<MultiElementId>;
    MultiElementIds _multiElementIds;

public:
    using SetId = T;

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

    SetId add(SetId setId, T elementId)
    {
        Q_ASSERT(!elementId.isNull());

        if(setId.isNull())
            setId = elementId;

        T lowId, highId;
        std::tie(lowId, highId) = std::minmax(setId, elementId);
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

    SetId remove(SetId setId, T elementId)
    {
        Q_ASSERT(!elementId.isNull());
        auto& multiElementId = _multiElementIds[elementId];

        // Can't remove it if it isn't a multielement
        if(multiElementId.isNull())
            return setId;

        if(multiElementId.isSingleton(elementId))
        {
            setId.setToNull();
        }
        else if(multiElementId._next == multiElementId._opposite)
        {
            // The tail is the only other element
            auto& tail = _multiElementIds[multiElementId._next];
            tail.setToSingleton(multiElementId._next);
            setId = multiElementId._next;
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
            setId = multiElementId._next;
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
            // Removing from the middle
            Q_ASSERT(!multiElementId._prev.isNull());
            Q_ASSERT(!multiElementId._next.isNull());
            auto& prev = _multiElementIds[multiElementId._prev];
            auto& next = _multiElementIds[multiElementId._next];

            prev._next = multiElementId._next;
            next._prev = multiElementId._prev;
        }

        multiElementId.setToNull();

        return setId;
    }

    SetId remove(T elementId)
    {
        return remove(SetId(), elementId);
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
};

using NodeIdDistinctSetCollection = ElementIdDistinctSetCollection<NodeId>;
using EdgeIdDistinctSetCollection = ElementIdDistinctSetCollection<EdgeId>;

template<typename C, typename T> class ElementIdDistinctSet
{
    static_assert(std::is_same<ElementIdDistinctSetCollection<T>, typename std::remove_const<C>::type>::value,
                  "C must be an ElementIdDistinctSetCollection");

    friend class ElementIdDistinctSets<T>;

private:
    T _head;
    C* _collection = nullptr;
    mutable int _size = -1;

public:
    ElementIdDistinctSet() : _size(0)
    {}

    ElementIdDistinctSet(T head, C* collection) :
        _head(head),
        _collection(collection)
    {}

    void setCollection(C* collection)
    {
        _collection = collection;
    }

    void add(T elementId)
    {
        _head = _collection->add(_head, elementId);

        if(_size >= 0)
            _size++;
    }

    void remove(T elementId)
    {
        _head = _collection->remove(_head, elementId);

        if(_size > 0)
            _size--;
    }

    class iterator_base
    {
    public:
        using self_type = iterator_base;
        using value_type = T;
        using reference = T;
        using pointer = T;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = int;

    protected:
        pointer _p;

    private:
        const ElementIdDistinctSet* _set = nullptr;

        const typename ElementIdDistinctSetCollection<T>::MultiElementId& multiElementId() const
        {
            return _set->_collection->_multiElementIds[_p];
        }

        void incrementPointer()
        {
            if(multiElementId().hasNext(_p))
                _p = multiElementId()._next;
            else
                _p.setToNull();
        }

    public:
        iterator_base() {}

        iterator_base(const ElementIdDistinctSet* set) :
             _set(set)
        {
            _p = _set->_head;
        }

        self_type operator++()
        {
            self_type i = *this;
            incrementPointer();
            return i;
        }

        bool operator!=(const self_type& other) { return _p != other._p; }
    };

    class iterator : public iterator_base
    {
    public:
#if __cplusplus >= 201103L
        using iterator_base::iterator_base;
#else
        iterator() : iterator_base() {}
        iterator(const ElementIdDistinctSet* set) : iterator_base(set) {}
#endif

        typename iterator_base::reference operator*() { return this->_p; }
    };

    class const_iterator : public iterator_base
    {
    public:
#if __cplusplus >= 201103L
        using iterator_base::iterator_base;
#else
        const_iterator() : iterator_base() {}
        const_iterator(const ElementIdDistinctSet* set) : iterator_base(set) {}
#endif

        const typename iterator_base::reference operator*() const { return this->_p; }
    };

    iterator begin() { return iterator(this); }
    iterator end()   { return iterator(); }

    const_iterator begin() const { return const_iterator(this); }
    const_iterator end() const   { return const_iterator(); }

    int size() const
    {
        if(_size < 0)
        {
            // If we don't know the size, calculate it on demand
            _size = u::count(*this);
        }

        return _size;
    }

    std::vector<T> copy() const
    {
        std::vector<T> v;

        std::copy(begin(), end(), std::back_inserter(v));

        return v;
    }
};

using NodeIdDistinctSet = ElementIdDistinctSet<ElementIdDistinctSetCollection<NodeId>, NodeId>;
using EdgeIdDistinctSet = ElementIdDistinctSet<ElementIdDistinctSetCollection<EdgeId>, EdgeId>;

using ConstNodeIdDistinctSet = ElementIdDistinctSet<const ElementIdDistinctSetCollection<NodeId>, NodeId>;
using ConstEdgeIdDistinctSet = ElementIdDistinctSet<const ElementIdDistinctSetCollection<EdgeId>, EdgeId>;

template<typename C, typename T> QDebug operator<<(QDebug d, const ElementIdDistinctSet<C, T>& set)
{
    d << "[";
    for(auto id : set)
        d << id;
    d << "]";

    return d;
}

template<typename T> class ElementIdDistinctSets
{
private:
    std::vector<const ElementIdDistinctSet<ElementIdDistinctSetCollection<T>, T>*> _sets;
    mutable int _size = -1;

public:
    ElementIdDistinctSets() : _size(0)
    {}

    void add(const ElementIdDistinctSet<ElementIdDistinctSetCollection<T>, T>& set)
    {
        _sets.push_back(&set);
        _size += set._size;
    }

    class iterator_base
    {
    public:
        using self_type = iterator_base;
        using value_type = T;
        using reference = T;
        using pointer = T;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = int;

    protected:
        pointer _p;

    private:
        const ElementIdDistinctSets* _sets = nullptr;
        int _i = 0;

        const typename ElementIdDistinctSetCollection<T>::MultiElementId& multiElementId() const
        {
            return _sets->_sets[_i]->_collection->_multiElementIds[_p];
        }

        pointer nextHead()
        {
            pointer p;
            while(_i < static_cast<int>(_sets->_sets.size()))
            {
                p = _sets->_sets[_i]->_head;
                if(p.isNull())
                    _i++;
                else
                    break;
            }

            return p;
        }

        void incrementPointer()
        {
            if(!multiElementId().hasNext(_p))
            {
                _i++;
                _p = nextHead();
            }
            else
                _p = multiElementId()._next;
        }

    public:
        iterator_base() {}

        iterator_base(const ElementIdDistinctSets* sets) :
             _sets(sets)
        {
            _p = nextHead();
        }

        self_type operator++()
        {
            self_type i = *this;
            incrementPointer();
            return i;
        }

        bool operator!=(const self_type& other) { return _p != other._p; }
    };

    class iterator : public iterator_base
    {
    public:
#if __cplusplus >= 201103L
        using iterator_base::iterator_base;
#else
        iterator() : iterator_base() {}
        iterator(const ElementIdDistinctSets* sets) : iterator_base(sets) {}
#endif

        typename iterator_base::reference operator*() { return this->_p; }
    };

    class const_iterator : public iterator_base
    {
    public:
#if __cplusplus >= 201103L
        using iterator_base::iterator_base;
#else
        const_iterator() : iterator_base() {}
        const_iterator(const ElementIdDistinctSets* sets) : iterator_base(sets) {}
#endif

        const typename iterator_base::reference operator*() const { return this->_p; }
    };

    iterator begin() { return iterator(this); }
    iterator end()   { return iterator(); }

    const_iterator begin() const { return const_iterator(this); }
    const_iterator end() const   { return const_iterator(); }

    int size() const
    {
        if(_size < 0)
        {
            // If we don't know the size, calculate it on demand
            _size = u::count(*this);
        }

        return _size;
    }

    std::vector<T> copy() const
    {
        std::vector<T> v;

        std::copy(begin(), end(), std::back_inserter(v));

        return v;
    }
};

using NodeIdDistinctSets = ElementIdDistinctSets<NodeId>;
using EdgeIdDistinctSets = ElementIdDistinctSets<EdgeId>;

template<typename T> QDebug operator<<(QDebug d, const ElementIdDistinctSets<T>& set)
{
    d << "[";
    for(auto id : set)
        d << id;
    d << "]";

    return d;
}

#endif // ELEMENTIDSETCOLLECTION

