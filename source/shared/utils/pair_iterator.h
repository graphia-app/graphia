#ifndef PAIR_ITERATOR_H
#define PAIR_ITERATOR_H

#include <iterator>

template<typename It, typename T, T It::value_type::*member>
class pair_iterator : public std::iterator<std::bidirectional_iterator_tag, T>
{
public:
    explicit pair_iterator(const It& it) : _it(it) {}
    auto& operator++() { return _it.operator++(); }
    auto operator++(int i) { return operator++(i); }
    auto& operator--() { return _it.operator--(); }
    auto operator--(int i) { return operator--(i); }
    bool operator==(const pair_iterator& other) const { return _it == other._it; }
    bool operator!=(const pair_iterator& other) const { return _it != other._it; }
    auto operator*() { return (*_it).*member; }
    auto operator->() { return &((*_it).*member); }

protected:
    It _it;
};

template<typename It> using pair_first_iterator =
    pair_iterator<It, decltype(It::value_type::first), &It::value_type::first>;
template<typename It> using pair_second_iterator =
    pair_iterator<It, decltype(It::value_type::second), &It::value_type::second>;

template<typename It>
auto make_pair_first_iterator(const It& it) { return pair_first_iterator<It>(it); }
template<typename It>
auto make_pair_second_iterator(const It& it) { return pair_second_iterator<It>(it); }

template<typename It>
auto make_map_key_iterator(const It& it) { return make_pair_first_iterator(it); }
template<typename It>
auto make_map_value_iterator(const It& it) { return make_pair_second_iterator(it); }

template<typename C>
class key_wrapper
{
public:
    explicit key_wrapper(const C& c) : _c(&c) {}
    auto begin() const { return make_map_key_iterator(_c->begin()); }
    auto end() const   { return make_map_key_iterator(_c->end()); }

protected:
    const C* _c;
};

template<typename C>
class value_wrapper
{
public:
    explicit value_wrapper(const C& c) : _c(&c) {}
    auto begin() { return make_map_value_iterator(_c->begin()); }
    auto end()   { return make_map_value_iterator(_c->end()); }

protected:
    const C* _c;
};

#endif // PAIR_ITERATOR_H
