/* Copyright © 2013-2024 Graphia Technologies Ltd.
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

#include <boost/iterator/iterator_adaptor.hpp>

#include <functional>

template<typename BaseItType>
class progress_iterator :
        public boost::iterator_adaptor<progress_iterator<BaseItType>, BaseItType>
{
private:
    std::function<void(size_t position)> _onPositionChangedFn;
    std::function<bool()> _cancelledFn;

    size_t _position = 0;

public:
    progress_iterator() = default;

    explicit progress_iterator(const BaseItType& iterator) :
        progress_iterator<BaseItType>::iterator_adaptor_(iterator)
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

private:
    friend class boost::iterator_core_access;

    void increment()
    {
        this->base_reference()++;
        _position++;

        if(_onPositionChangedFn != nullptr)
            _onPositionChangedFn(_position);

        if(_cancelledFn != nullptr && _cancelledFn())
        {
            // Reset the underlying iterator, effectively making it eof
            this->base_reference() = BaseItType();
        }
    }
};

#endif // PROGRESS_ITERATOR_H
