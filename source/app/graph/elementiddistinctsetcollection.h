/* Copyright © 2013-2023 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ELEMENTIDDISTINCTSETCOLLECTION
#define ELEMENTIDDISTINCTSETCOLLECTION

#include "shared/graph/elementid.h"

#include <vector>
#include <array>
#include <type_traits>
#include <iterator>
#include <algorithm>
#include <cassert>

enum class MultiElementType
{
    Not,
    Head,
    Tail
};

template<typename C, typename T> class ElementIdDistinctSet;
template<typename T> class ElementIdDistinctSets;

template<typename T> class ElementIdDistinctSetCollection
{
    static_assert(std::is_base_of_v<ElementId<T>, T>, "T must be an ElementId");

    friend class ElementIdDistinctSet<ElementIdDistinctSetCollection<T>, T>;
    friend class ElementIdDistinctSet<const ElementIdDistinctSetCollection<T>, T>;
    friend class ElementIdDistinctSets<ElementIdDistinctSet<ElementIdDistinctSetCollection<T>, T>>;
    friend class ElementIdDistinctSets<ElementIdDistinctSet<const ElementIdDistinctSetCollection<T>, T>>;

private:
    struct ListNode
    {
        // Null: none of prev, next or opposite is set
        // Tail: next is self, opposite set to the head
        // Head: prev is not set, has an opposite and (next isn't self or opposite is self)
        // Middle: prev and next are set
        // Singleton: prev, next and opposite are self

        T _prev;
        T _next;
        T _opposite;

        bool isNull() const { return _next.isNull(); }
        bool isTail(T elementId) const { return !isNull() && _next == elementId; }
        bool isHead(T elementId) const { return !isNull() && !_opposite.isNull() && (!isTail(elementId) || _opposite == elementId); }
        bool isSingleton(T elementId) const { return isHead(elementId) && isTail(elementId); }

        void setToNull() { _prev.setToNull(); _next.setToNull(); _opposite.setToNull(); }
        void setToSingleton(T elementId) { _prev = _next = _opposite = elementId; }

        bool hasNext(T elementId) const { return !_next.isNull() && !isTail(elementId); }
    };

    using List = std::vector<ListNode>;
    List _list;

    const ListNode& listNodeFor(T elementId) const
    {
        assert(!elementId.isNull());
        return _list.at(static_cast<size_t>(elementId));
    }

    ListNode& listNodeFor(T elementId)
    {
        assert(!elementId.isNull());
        return _list.at(static_cast<size_t>(elementId));
    }

public:
    using SetId = T;

    void resize(std::size_t size)
    {
        _list.resize(size);
    }

    void clear()
    {
        _list.clear();
    }

    SetId add(SetId setId, T elementId)
    {
        assert(!elementId.isNull());

        if(setId.isNull())
        {
            assert(listNodeFor(elementId).isNull() || listNodeFor(elementId).isSingleton(elementId));
            setId = elementId;
        }

        auto [lowId, highId] = std::minmax(setId, elementId);
        auto& lowListNode = listNodeFor(lowId);
        auto& highListNode = listNodeFor(highId);

        if(lowListNode.isSingleton(lowId))
            lowListNode.setToNull();

        if(highListNode.isSingleton(highId))
            highListNode.setToNull();

        if(lowListNode.isNull() && highListNode.isNull())
        {
            // Neither is yet merged with anything
            lowListNode._next = highId;
            lowListNode._opposite = highId;

            highListNode._prev = lowId;
            highListNode._next = highId;
            highListNode._opposite = lowId;
        }
        else if(lowId != highId)
        {
            // Don't merge if they're the same list

            if(lowListNode.isHead(lowId) && highListNode.isHead(highId))
            {
                // Merge two existing lists together
                auto& tailOne = listNodeFor(lowListNode._opposite);
                tailOne._opposite.setToNull();
                tailOne._next = highId;

                auto& tailTwo = listNodeFor(highListNode._opposite);
                tailTwo._opposite = lowId;

                highListNode._prev = lowListNode._opposite;
                lowListNode._opposite = highListNode._opposite;

                highListNode._opposite.setToNull();
            }
            else if(highListNode.isHead(highId))
            {
                assert(!highListNode._opposite.isNull());
                assert(lowListNode.isNull());
                assert(lowListNode._prev.isNull());
                assert(lowListNode._opposite.isNull());

                // Adding to the head
                auto& tail = listNodeFor(highListNode._opposite);
                tail._opposite = lowId;

                lowListNode._next = highId;
                lowListNode._opposite = highListNode._opposite;

                highListNode._prev = lowId;
                highListNode._opposite.setToNull();
            }
            else if(lowListNode.isTail(lowId))
            {
                assert(!lowListNode._opposite.isNull());
                assert(highListNode.isNull());
                assert(highListNode._prev.isNull());
                assert(highListNode._opposite.isNull());

                // Adding to the tail
                auto& head = listNodeFor(lowListNode._opposite);
                head._opposite = highId;

                highListNode._prev = lowId;
                highListNode._next = highId;
                highListNode._opposite = lowListNode._opposite;

                lowListNode._next = highId;
                lowListNode._opposite.setToNull();
            }
            else
            {
                // Adding in the middle
                if(!lowListNode.isNull())
                {
                    assert(highListNode.isNull());
                    assert(!lowListNode._next.isNull());
                    auto& next = listNodeFor(lowListNode._next);

                    highListNode._prev = lowId;
                    highListNode._next = lowListNode._next;

                    lowListNode._next = highId;
                    next._prev = highId;
                }
                else if(!highListNode.isNull())
                {
                    assert(lowListNode.isNull());
                    assert(!highListNode._prev.isNull());
                    auto& prev = listNodeFor(highListNode._prev);

                    lowListNode._prev = highListNode._prev;
                    lowListNode._next = highId;

                    highListNode._prev = lowId;
                    prev._next = lowId;
                }
            }
        }

        return lowId;
    }

    SetId remove(SetId setId, T elementId)
    {
        assert(!elementId.isNull());
        auto& listNode = listNodeFor(elementId);

        // Can't remove it if it isn't in the list
        if(listNode.isNull())
            return SetId();

        if(listNode.isSingleton(elementId))
        {
            setId.setToNull();
        }
        else if(listNode._next == listNode._opposite)
        {
            // The tail is the only other element
            auto& tail = listNodeFor(listNode._next);
            tail.setToSingleton(listNode._next);
            setId = listNode._next;

            assert(listNodeFor(setId).isHead(setId) && listNodeFor(setId).isSingleton(setId));
        }
        else if(listNode._prev == listNode._opposite)
        {
            // The head is the only other element
            auto& head = listNodeFor(listNode._prev);
            head.setToSingleton(listNode._prev);
            setId = listNode._prev;

            assert(listNodeFor(setId).isHead(setId) && listNodeFor(setId).isSingleton(setId));
        }
        else if(listNode.isHead(elementId))
        {
            // Removing from the head
            assert(!listNode._next.isNull());
            assert(!listNode._opposite.isNull());
            auto& newHead = listNodeFor(listNode._next);
            auto& tail = listNodeFor(listNode._opposite);

            newHead._opposite = listNode._opposite;
            newHead._prev.setToNull();
            tail._opposite = listNode._next;
            setId = listNode._next;

            assert(listNodeFor(setId).isHead(setId));
        }
        else if(listNode.isTail(elementId))
        {
            // Removing from the tail
            assert(!listNode._opposite.isNull());
            assert(!listNode._prev.isNull());
            auto& head = listNodeFor(listNode._opposite);
            auto& newTail = listNodeFor(listNode._prev);

            head._opposite = listNode._prev;
            newTail._next = listNode._prev;
            newTail._opposite = listNode._opposite;
            setId = listNode._opposite;

            assert(listNodeFor(setId).isHead(setId));
        }
        else
        {
            // Removing from the middle
            assert(!listNode._prev.isNull());
            assert(!listNode._next.isNull());
            auto& prev = listNodeFor(listNode._prev);
            auto& next = listNodeFor(listNode._next);

            prev._next = listNode._next;
            next._prev = listNode._prev;
        }

        listNode.setToNull();

        assert(setId.isNull() || listNodeFor(setId).isNull() || listNodeFor(setId).isHead(setId));

        return setId;
    }

    MultiElementType typeOf(T elementId) const
    {
        assert(!elementId.isNull());
        auto& listNode = listNodeFor(elementId);

        if(!listNode.isNull() && !listNode.isSingleton(elementId))
        {
            if(listNode.isHead(elementId))
                return MultiElementType::Head;

            return MultiElementType::Tail;
        }

        return MultiElementType::Not;
    }
};

using NodeIdDistinctSetCollection = ElementIdDistinctSetCollection<NodeId>;
using EdgeIdDistinctSetCollection = ElementIdDistinctSetCollection<EdgeId>;

template<typename C, typename T> class ElementIdDistinctSet
{
    static_assert(std::is_same_v<ElementIdDistinctSetCollection<T>, typename std::remove_const_t<C>>,
                  "C must be an ElementIdDistinctSetCollection");

    friend class ElementIdDistinctSets<ElementIdDistinctSet<C, T>>;
    friend class ElementIdDistinctSets<ElementIdDistinctSet<const C, T>>;

private:
    T _head;
    C* _collection = nullptr;
    mutable int _size = -1;

public:
    using value_type = T;

    ElementIdDistinctSet() : _size(0)
    {}

    // Construct empty set, with no head yet
    explicit ElementIdDistinctSet(C* collection) :
        _collection(collection),
        _size(0)
    {}

    // Construct set from pre-existing head
    ElementIdDistinctSet(T head, C* collection) :
        _head(head),
        _collection(collection)
    {
        assert(collection->typeOf(head) != MultiElementType::Tail);
    }

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

        assert(_size == -1 || _size > 0);
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

        const typename ElementIdDistinctSetCollection<T>::ListNode& listNode() const
        {
            return _set->_collection->listNodeFor(_p);
        }

        void incrementPointer()
        {
            if(listNode().hasNext(_p))
                _p = listNode()._next;
            else
                _p.setToNull();
        }

    public:
        iterator_base() = default;

        explicit iterator_base(const ElementIdDistinctSet* set) :
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

        bool operator!=(const self_type& other) const { return _p != other._p; }
        bool operator==(const self_type& other) const { return _p == other._p; }
    };

    class iterator : public iterator_base
    {
    public:
        using iterator_base::iterator_base;
        typename iterator_base::reference operator*() const { return this->_p; }
    };

    class const_iterator : public iterator_base
    {
    public:
        using iterator_base::iterator_base;
        typename iterator_base::reference operator*() const { return this->_p; }
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
            _size = std::distance(begin(), end());
        }

        return _size;
    }

    bool empty() const { return size() <= 0; }

    std::vector<T> copy() const
    {
        std::vector<T> v;
        v.reserve(static_cast<size_t>(_size));

        std::copy(begin(), end(), std::back_inserter(v));

        return v;
    }
};

using NodeIdDistinctSet = ElementIdDistinctSet<ElementIdDistinctSetCollection<NodeId>, NodeId>;
using EdgeIdDistinctSet = ElementIdDistinctSet<ElementIdDistinctSetCollection<EdgeId>, EdgeId>;

using ConstNodeIdDistinctSet = ElementIdDistinctSet<const ElementIdDistinctSetCollection<NodeId>, NodeId>;
using ConstEdgeIdDistinctSet = ElementIdDistinctSet<const ElementIdDistinctSetCollection<EdgeId>, EdgeId>;

template<typename T> class ElementIdDistinctSets
{
private:
    // Using a vector causes malloc overhead so for small numbers of sets use an array instead
    std::array<const T*, 8> _setsSmall;
    std::vector<const T*> _setsBig;
    size_t _numSets = 0;
    mutable int _size = -1;

    const T* setAt(size_t index) const
    {
        if(index >= _numSets)
            return nullptr;

        if(index < _setsSmall.size())
            return _setsSmall.at(index);

        return _setsBig.at(index - _setsSmall.size());
    }

public:
    ElementIdDistinctSets() : _size(0)
    {
        _setsSmall.fill(nullptr);
    }

    void add(const T& set)
    {
        if(_numSets < _setsSmall.size())
            _setsSmall.at(_numSets++) = &set;
        else
        {
            _setsBig.push_back(&set);
            _numSets++;
        }

        _size += set._size;
    }

    class iterator_base
    {
    public:
        using self_type = iterator_base;
        using value_type = typename T::iterator_base::value_type;
        using reference = typename T::iterator_base::reference;
        using pointer = typename T::iterator_base::pointer;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = int;

    private:
        const ElementIdDistinctSets* _sets = nullptr;
        size_t _i = 0;

    protected:
        pointer _p;

    private:
        const typename ElementIdDistinctSetCollection<typename T::value_type>::ListNode& listNode() const
        {
            return _sets->setAt(_i)->_collection->listNodeFor(_p);
        }

        pointer nextHead()
        {
            pointer p;
            while(_i < _sets->_numSets)
            {
                p = _sets->setAt(_i)->_head;
                if(p.isNull())
                    _i++;
                else
                    break;
            }

            return p;
        }

        void incrementPointer()
        {
            if(!listNode().hasNext(_p))
            {
                _i++;
                _p = nextHead();
            }
            else
                _p = listNode()._next;
        }

    public:
        iterator_base() = default;

        explicit iterator_base(const ElementIdDistinctSets* sets) :
            _sets(sets), _p(nextHead())
        {}

        self_type operator++()
        {
            self_type i = *this;
            incrementPointer();
            return i;
        }

        bool operator!=(const self_type& other) const { return _p != other._p; }
        bool operator==(const self_type& other) const { return _p == other._p; }
    };

    class iterator : public iterator_base
    {
    public:
        using iterator_base::iterator_base;
        typename iterator_base::reference operator*() const { return this->_p; }
    };

    class const_iterator : public iterator_base
    {
    public:
        using iterator_base::iterator_base;
        typename iterator_base::reference operator*() const { return this->_p; }
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
            _size = std::distance(begin(), end());
        }

        return _size;
    }

    bool empty() const { return size() <= 0; }

    std::vector<typename T::value_type> copy() const
    {
        std::vector<typename T::value_type> v;
        v.reserve(static_cast<size_t>(_size));

        std::copy(begin(), end(), std::back_inserter(v));

        return v;
    }
};

using NodeIdDistinctSets = ElementIdDistinctSets<NodeIdDistinctSet>;
using EdgeIdDistinctSets = ElementIdDistinctSets<EdgeIdDistinctSet>;

using ConstNodeIdDistinctSets = ElementIdDistinctSets<ConstNodeIdDistinctSet>;
using ConstEdgeIdDistinctSets = ElementIdDistinctSets<ConstEdgeIdDistinctSet>;

#endif // ELEMENTIDSETCOLLECTION

