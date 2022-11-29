/* Copyright © 2013-2022 Graphia Technologies Ltd.
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

#ifndef PAIR_ITERATOR_H
#define PAIR_ITERATOR_H

#include <iterator>
#include <utility>
#include <cstddef>

template<typename Iterator, typename T, T Iterator::value_type::*member>
class pair_iterator
{
public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = T;
    using difference_type = ptrdiff_t;
    using pointer = T*;
    using reference = T&;

    explicit pair_iterator(Iterator it) : _it(std::move(it)) {}
    auto& operator++() { return _it.operator++(); }
    auto operator++(int i) { return operator++(i); }
    auto& operator--() { return _it.operator--(); }
    auto operator--(int i) { return operator--(i); }
    bool operator==(const pair_iterator& other) const { return _it == other._it; }
    bool operator!=(const pair_iterator& other) const { return _it != other._it; }
    auto& operator*() { return (*_it).*member; }
    auto operator->() { return &((*_it).*member); }

private:
    Iterator _it;
};

template<typename Iterator> using pair_first_iterator =
    pair_iterator<Iterator, decltype(Iterator::value_type::first), &Iterator::value_type::first>;
template<typename Iterator> using pair_second_iterator =
    pair_iterator<Iterator, decltype(Iterator::value_type::second), &Iterator::value_type::second>;

template<typename Iterator>
auto make_pair_first_iterator(const Iterator& it) { return pair_first_iterator<Iterator>(it); }
template<typename Iterator>
auto make_pair_second_iterator(const Iterator& it) { return pair_second_iterator<Iterator>(it); }

template<typename Iterator>
auto make_map_key_iterator(const Iterator& it) { return make_pair_first_iterator(it); }
template<typename Iterator>
auto make_map_value_iterator(const Iterator& it) { return make_pair_second_iterator(it); }

template<typename C>
class key_wrapper
{
public:
    explicit key_wrapper(C& c) : _c(&c) {}
    auto begin() const { return make_map_key_iterator(_c->begin()); }
    auto end() const   { return make_map_key_iterator(_c->end()); }

private:
    C* _c;
};

template<typename C>
auto make_key_wrapper(C& c) { return key_wrapper<C>(c); }

template<typename C>
class value_wrapper
{
public:
    explicit value_wrapper(C& c) : _c(&c) {}
    auto begin() const { return make_map_value_iterator(_c->begin()); }
    auto end() const   { return make_map_value_iterator(_c->end()); }

private:
    C* _c;
};

template<typename C>
auto make_value_wrapper(C& c) { return value_wrapper<C>(c); }

#endif // PAIR_ITERATOR_H
