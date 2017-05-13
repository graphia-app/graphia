#ifndef ITERATOR_RANGE_H
#define ITERATOR_RANGE_H

#include <type_traits>

template<typename BeginIt, typename EndIt>
class iterator_range
{
public:
    iterator_range(BeginIt begin, EndIt end) :
        _begin(begin), _end(end) {}

    BeginIt& begin() const { return _begin; }
    EndIt& end() const { return _end; }

private:
    BeginIt _begin;
    EndIt _end;
};

// This converts an rvalue reference to a value,
// and everything else to an lvalue reference
template<typename T>
using rv2v = std::conditional_t
<
    std::is_rvalue_reference<T>::value,
    std::remove_reference_t<T>,
    std::add_lvalue_reference_t<std::decay_t<T>>
>;

template<typename BeginIt, typename EndIt>
auto make_iterator_range(BeginIt&& begin, EndIt&& end)
{
    return iterator_range<rv2v<BeginIt>, rv2v<EndIt>>(begin, end);
}

#endif // ITERATOR_RANGE_H
