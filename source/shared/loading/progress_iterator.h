/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

#ifndef PROGRESS_ITERATOR_H
#define PROGRESS_ITERATOR_H

#include <cstddef>
#include <functional>
#include <iterator>

// A forward iterator that wraps another iterator, periodically reporting how far
// it has advanced, and which can be made to appear as if it has reached the end,
// thereby terminating whatever is consuming it
//
// Note this is deliberately only a forward iterator; were it to model a random
// access iterator, std::advance and friends would bypass the increment operator,
// and no progress would ever be reported
template<typename BaseItType>
class progress_iterator
{
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = typename std::iterator_traits<BaseItType>::value_type;
    using difference_type = typename std::iterator_traits<BaseItType>::difference_type;
    using pointer = typename std::iterator_traits<BaseItType>::pointer;
    using reference = typename std::iterator_traits<BaseItType>::reference;

private:
    // Number of increments between successive polls; calling out on every single
    // one of them is needlessly expensive, given that neither the progress
    // indication nor the response to cancellation need be especially fine grained
    static constexpr size_t POLL_INTERVAL = 4096;

    BaseItType _it = {};

    std::function<void(size_t position)> _onPositionChangedFn;
    std::function<bool()> _cancelledFn;

    size_t _position = 0;
    size_t _poll = POLL_INTERVAL;
    bool _cancelled = false;

public:
    progress_iterator() = default;

    explicit progress_iterator(const BaseItType& iterator) :
        _it(iterator)
    {}

    template<typename OnPositionChangedFn>
    void onPositionChanged(const OnPositionChangedFn& onPositionChangedFn)
    {
        _onPositionChangedFn = onPositionChangedFn;
    }

    template<typename CancelledFn>
    void setCancelledFn(const CancelledFn& cancelledFn)
    {
        _cancelledFn = cancelledFn;
    }

    reference operator*() const { return *_it; }
    pointer operator->() const { return &(*_it); }

    progress_iterator& operator++()
    {
        increment();
        return *this;
    }

    progress_iterator operator++(int)
    {
        auto previous = *this;
        increment();
        return previous;
    }

    bool operator==(const progress_iterator& other) const
    {
        // Once cancelled, appear to be at the end, whatever the end may be
        return _cancelled || other._cancelled || _it == other._it;
    }

    bool operator!=(const progress_iterator& other) const { return !(*this == other); }

private:
    void increment()
    {
        _it++;
        _position++;

        if(_position < _poll)
            return;

        _poll = _position + POLL_INTERVAL;

        if(_onPositionChangedFn != nullptr)
            _onPositionChangedFn(_position);

        if(_cancelledFn != nullptr && _cancelledFn())
            _cancelled = true;
    }
};

#endif // PROGRESS_ITERATOR_H
