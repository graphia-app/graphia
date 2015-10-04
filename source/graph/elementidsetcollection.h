#ifndef ELEMENTIDSETCOLLECTION
#define ELEMENTIDSETCOLLECTION

#include "elementid.h"

#include <QDebug>

#include <vector>
#include <algorithm>

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

    class Set
    {
        friend class ElementIdSetCollection<T>;
        friend class MutableGraph;

    private:
        std::vector<std::pair<T, const MultiElementIds*>> _heads;

        Set(T head, const MultiElementIds* multiElementIds) :
            _heads({{head, multiElementIds}})
        {}

    public:
        Set() {}

        void add(const Set& other)
        {
            _heads.insert(_heads.end(), other._heads.begin(), other._heads.end());
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
            const Set* _set = nullptr;
            int _i = 0;

            const MultiElementId& multiElementId() const
            {
                return (*_set->_heads[_i].second)[_p];
            }

            pointer nextHead()
            {
                pointer p;
                while(_i < static_cast<int>(_set->_heads.size()))
                {
                    p = _set->_heads[_i].first;
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

            iterator_base(const Set* set) :
                 _set(set)
            {
                _p = nextHead();
            }

            self_type operator++()
            {
                self_type i = *this;
                incrementPointer();
                return i;
            }

            self_type operator++(int)
            {
                incrementPointer();
                return *this;
            }


            bool operator==(const self_type& other) { return _p == other._p; }
            bool operator!=(const self_type& other) { return _p != other._p; }
        };

        class iterator : public iterator_base
        {
        public:
#if __cplusplus >= 201103L
            using iterator_base::iterator_base;
#else
            iterator() : iterator_base() {}
            iterator(const Set* set) : iterator_base(set) {}
#endif

            typename iterator_base::reference operator*() { return this->_p; }
            typename iterator_base::pointer operator->() { return this->_p; }
        };

        class const_iterator : public iterator_base
        {
        public:
#if __cplusplus >= 201103L
            using iterator_base::iterator_base;
#else
            const_iterator() : iterator_base() {}
            const_iterator(const Set* set) : iterator_base(set) {}
#endif

            const typename iterator_base::reference operator*() const { return this->_p; }
            const typename iterator_base::pointer operator->() const { return this->_p; }
        };

        iterator begin() { return iterator(this); }
        iterator end()   { return iterator(); }

        const_iterator begin() const { return const_iterator(this); }
        const_iterator end() const   { return const_iterator(); }

        std::vector<T> copy()
        {
            std::vector<T> v;

            std::copy(begin(), end(), std::back_inserter(v));

            return v;
        }
    };

    Set setById(SetId setId) const
    {
        return Set(setId, &_multiElementIds);
    }

    template<typename C> Set setByIds(const C& setIds) const
    {
        Set set;

        for(auto setId : setIds)
            set.add(setById(setId));

        return set;
    }
};

using NodeIdSetCollection = ElementIdSetCollection<NodeId>;
using EdgeIdSetCollection = ElementIdSetCollection<EdgeId>;

QDebug operator<<(QDebug d, NodeIdSetCollection::Set& set);
QDebug operator<<(QDebug d, EdgeIdSetCollection::Set& set);

#endif // ELEMENTIDSETCOLLECTION

