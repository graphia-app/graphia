#ifndef PROGRESS_ITERATOR_H
#define PROGRESS_ITERATOR_H

#include "thirdparty/boost/boost_disable_warnings.h"
#include "boost/iterator/iterator_adaptor.hpp"
#include "thirdparty/boost/boost_spirit_qstring_adapter.h"

#include <functional>

template<typename BaseItType>
class progress_iterator :
        public boost::iterator_adaptor<progress_iterator<BaseItType>, BaseItType>
{
private:
    std::function<void(size_t position)> _onPositionChangedFn;
    std::function<bool()> _cancelledFn;

    const progress_iterator<BaseItType>* _end = nullptr;
    size_t _position = 0;

public:
    progress_iterator() : progress_iterator<BaseItType>::iterator_adaptor_() {}

    progress_iterator(const BaseItType& iterator, const progress_iterator<BaseItType>& end) :
        progress_iterator<BaseItType>::iterator_adaptor_(iterator),
        _end(&end)
    {}

    template<typename OnPositionChangedFn>
    void onPositionChanged(OnPositionChangedFn&& onPositionChangedFn)
    {
        _onPositionChangedFn = onPositionChangedFn;
    }

    template<typename CancelledFn>
    void setCancelledFn(CancelledFn&& cancelledFn)
    {
        _cancelledFn = cancelledFn;
    }

private:
    friend class boost::iterator_core_access;

    bool equal(const progress_iterator& other) const
    {
        // Check if we've been cancelled
        if(_cancelledFn != nullptr && _cancelledFn())
        {
            if(_end != nullptr && other == *_end)
            {
                // If we're testing against the end iterator unconditionally return
                // true, thus tricking whatever is using the iterator into
                // believing that we have actually reached the end
                return true;
            }
        }

        return this->base_reference() == other.base_reference();
    }

    void increment()
    {
        this->base_reference()++;
        _position++;

        if(_onPositionChangedFn != nullptr)
            _onPositionChangedFn(_position);
    }
};

#endif // PROGRESS_ITERATOR_H
