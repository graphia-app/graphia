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

#ifndef IS_STD_CONTAINER_H
#define IS_STD_CONTAINER_H

// https://stackoverflow.com/questions/9407367

#include <deque>
#include <forward_list>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

// Specialize a type for all of the std containers
namespace is_std_container_impl
{
    template<typename T>       struct is_std_container:std::false_type{};
    template<typename T, std::size_t N> struct is_std_container<std::array    <T,N>>    :std::true_type{};
    template<typename... Args> struct is_std_container<std::vector            <Args...>>:std::true_type{};
    template<typename... Args> struct is_std_container<std::deque             <Args...>>:std::true_type{};
    template<typename... Args> struct is_std_container<std::list              <Args...>>:std::true_type{};
    template<typename... Args> struct is_std_container<std::forward_list      <Args...>>:std::true_type{};
    template<typename... Args> struct is_std_container<std::set               <Args...>>:std::true_type{};
    template<typename... Args> struct is_std_container<std::multiset          <Args...>>:std::true_type{};
    template<typename... Args> struct is_std_container<std::map               <Args...>>:std::true_type{};
    template<typename... Args> struct is_std_container<std::multimap          <Args...>>:std::true_type{};
    template<typename... Args> struct is_std_container<std::unordered_set     <Args...>>:std::true_type{};
    template<typename... Args> struct is_std_container<std::unordered_multiset<Args...>>:std::true_type{};
    template<typename... Args> struct is_std_container<std::unordered_map     <Args...>>:std::true_type{};
    template<typename... Args> struct is_std_container<std::unordered_multimap<Args...>>:std::true_type{};
    template<typename... Args> struct is_std_container<std::stack             <Args...>>:std::true_type{};
    template<typename... Args> struct is_std_container<std::queue             <Args...>>:std::true_type{};
    template<typename... Args> struct is_std_container<std::priority_queue    <Args...>>:std::true_type{};
} // namespace is_std_container_impl

template<typename T> struct is_std_container
{
    static constexpr const bool value = is_std_container_impl::is_std_container<std::decay_t<T>>::value;
};

template<typename T>
inline constexpr bool is_std_container_v = is_std_container<T>::value;

// Specialize a type for all of the std sequence containers
namespace is_std_sequence_container_impl
{
    template<typename T>       struct is_std_sequence_container:std::false_type{};
    template<typename T, std::size_t N> struct is_std_sequence_container<std::array    <T,N>>    :std::true_type{};
    template<typename... Args> struct is_std_sequence_container<std::vector            <Args...>>:std::true_type{};
    template<typename... Args> struct is_std_sequence_container<std::deque             <Args...>>:std::true_type{};
    template<typename... Args> struct is_std_sequence_container<std::list              <Args...>>:std::true_type{};
    template<typename... Args> struct is_std_sequence_container<std::forward_list      <Args...>>:std::true_type{};
} // namespace is_std_sequence_container_impl

template<typename T> struct is_std_sequence_container
{
    static constexpr const bool value = is_std_sequence_container_impl::is_std_sequence_container<std::decay_t<T>>::value;
};

template<typename T>
inline constexpr bool is_std_sequence_container_v = is_std_sequence_container<T>::value;

#endif // IS_STD_CONTAINER_H
