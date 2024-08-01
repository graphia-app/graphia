#ifndef CONTAINER_COMBINE_H
#define CONTAINER_COMBINE_H

#include <iterator>
#include <tuple>

namespace u
{
template<typename... Containers>
class combine
{
private:
    template<typename T>
    using ContainerIterator = decltype(std::cbegin(std::declval<T&>()));

    template<typename T>
    using ContainerIteratorPair = std::pair<ContainerIterator<T>, ContainerIterator<T>>;

    template<typename>
    struct iterator_pairs {};

    template<typename... Ts>
    struct iterator_pairs<std::tuple<Ts...>>
    {
        using type = std::tuple<ContainerIteratorPair<Ts>...>;
    };

    using ContainerTypes = std::tuple<Containers...>;
    using ContainerIteratorPairs = typename iterator_pairs<ContainerTypes>::type;

    // A tuple of pairs of begin/end iterators
    ContainerIteratorPairs _its;

public:
    explicit combine(Containers&... cs) :
        _its(std::apply([](auto&&... values)
        {
            return std::tuple{(std::pair{std::cbegin(values), std::cend(values)})...};
        }, std::forward_as_tuple(cs...)))
    {}

    combine(const Containers&&... cs) = delete;

    using iterator_type = typename std::tuple_element_t<0, ContainerIteratorPairs>::first_type;
    using value_type = typename std::iterator_traits<iterator_type>::value_type;

    class iterator
    {
        friend class combine;

    private:
        combine::ContainerIteratorPairs _its;

        iterator(combine::ContainerIteratorPairs its, bool end) : _its(its)
        {
            if(end)
                std::apply([](auto&&... it) { ((it.first = it.second), ...); }, _its);
        }

    public:
        using self_type = iterator;
        using value_type = combine::value_type;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = size_t;

        self_type operator++()
        {
            self_type i = *this;

            std::apply([](auto&&... it)
            {
                (void)((it.first != it.second ? it.first++, true : false) || ...);
            }, _its);

            return i;
        }

        const auto& operator*() const
        {
            const value_type* v = nullptr;

            std::apply([&v](auto&&... it)
            {
                (void)((it.first != it.second ? v = &(*it.first), true : false) || ...);
            }, _its);

            return *v;
        }

        bool operator!=(const self_type& other) const { return !operator==(other); }
        bool operator==(const self_type& other) const { return _its == other._its; }
    };

    iterator begin() { return iterator(_its, false); }
    iterator end() { return iterator(_its, true); }
};
} // namespace u

#endif // CONTAINER_COMBINE_H
