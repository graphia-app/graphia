#ifndef ITERATOR_RANGE_H
#define ITERATOR_RANGE_H

#include <type_traits>
#include <utility>

template<typename Iterator>
struct is_const_iterator
{
    using pointer = typename std::iterator_traits<Iterator>::pointer;
    static const bool value =
        std::is_const<typename std::remove_pointer<pointer>::type>::value;
};

template<typename BeginIt, typename EndIt> class iterator_range
{
public:
    iterator_range(BeginIt begin_, EndIt end_) :
        _begin(begin_), _end(end_) {}

    BeginIt& begin() { return _begin; }
    EndIt& end() { return _end; }

    template<typename T = BeginIt> typename std::enable_if_t<is_const_iterator<T>::value, const T&>
    begin() const { return _begin; }

    template<typename T = BeginIt> typename std::enable_if_t<is_const_iterator<T>::value, const T&>
    end() const { return _end; }

    template<typename T = BeginIt> typename std::enable_if_t<is_const_iterator<T>::value, const T&>
    cbegin() const { return _begin; }

    template<typename T = BeginIt> typename std::enable_if_t<is_const_iterator<T>::value, const T&>
    cend() const { return _end; }

    auto size() const { return std::distance(begin(), end()); }

private:
    BeginIt _begin;
    EndIt _end;
};

template<typename BeginIt, typename EndIt>
auto make_iterator_range(BeginIt&& begin, EndIt&& end)
{
    return iterator_range<BeginIt, EndIt>(begin, end);
}

#endif // ITERATOR_RANGE_H
