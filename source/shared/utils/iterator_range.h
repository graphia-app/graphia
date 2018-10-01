#ifndef ITERATOR_RANGE_H
#define ITERATOR_RANGE_H

#include <type_traits>
#include <utility>

template<typename Iterator>
struct is_const_iterator
{
    using pointer = typename std::iterator_traits<Iterator>::pointer;
    static constexpr bool value =
        std::is_const_v<typename std::remove_pointer<pointer>::type>;
};

template<typename Iterator>
inline constexpr bool is_const_iterator_v = is_const_iterator<Iterator>::value;

template<typename BeginIt, typename EndIt> class iterator_range
{
public:
    iterator_range(BeginIt begin_, EndIt end_) :
        _begin(begin_), _end(end_) {}

    BeginIt& begin() { return _begin; }
    EndIt& end() { return _end; }

    template<typename T = BeginIt> typename std::enable_if_t<is_const_iterator_v<T>, const T&>
    begin() const { return _begin; }

    template<typename T = BeginIt> typename std::enable_if_t<is_const_iterator_v<T>, const T&>
    end() const { return _end; }

    template<typename T = BeginIt> typename std::enable_if_t<is_const_iterator_v<T>, const T&>
    cbegin() const { return _begin; }

    template<typename T = BeginIt> typename std::enable_if_t<is_const_iterator_v<T>, const T&>
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
